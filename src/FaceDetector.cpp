
#include "FaceDetector.h"
#include "Utils.h"
#include "ListWriter.h"


FaceDetector::FaceDetector(const std::string & inFile, const CutConfigByFrame& cutcfg)
	: _cutcfg(cutcfg),
    _vc(inFile),
    _detector(_vc)
{
    if (not _detector.videoCapture()->isOpened()) {
        std::cerr << "open video source failed." << std::endl;
    }
    std::cout << "fps: " << getFPS() << std::endl;
}

FaceDetector::FaceDetector(const std::string & inFile, const CutConfig& cutcfg)
	:
      _vc(inFile),
      _detector(_vc)
{
    if (not _detector.videoCapture()->isOpened()) {
        std::cerr << "open video source failed." << std::endl;
    }
    _cutcfg = CutConfigByFrame(cutcfg, _vc.get(cv::CAP_PROP_FPS));
}

bool
FaceDetector::detect() {
	// 检测人脸
    bool ret = _detector.detect();
    if (ret)
    {
        //cv::rectangle(frame, _detector.face(),
        //              cv::Scalar(255, 0, 0));
        //cv::circle(frame, _detector.facePosition(), 30,
        //           cv::Scalar(0, 255, 0));
        //cv::imshow("FVT", frame);
        //printf("Found face at %zu\n", getFramePos());
    }

	return ret;
}

#if 0
size_t FaceDetector::relocateFrame_R2L(bool to_faces_existed, size_t left, size_t right) {
	cv::Mat img;

	if (to_faces_existed) { // 直到发现脸
		while (right >= left) {
            img = ((*this) << 1).current();
			auto faces = detect(img);
			if (!faces.empty()) {
				return right;
			} 
			--right;
		}

	} else { // 直到脸消失
		while (right >= left) {
            img = ((*this) << 1).current();
			auto faces = detect(img);
			if (faces.empty()) {
				return right;
			}
			--right;
		}

	}

	return right;
}
#endif

void FaceDetector::run_range(size_t left, size_t right, size_t step) {
	ListWriter lw;
	size_t & frameCount = left;
	setFramePos(frameCount);

    bool flagAcceptNewFace = true;
    // 当前的帧计数、检测到人脸的最近一次帧计数、起始帧计数
    size_t lastFrameCount = 0, detectStartFrameCount = 0;
    double lastMs = 0., detectStartMs = 0.;

	while (left <= right) {

        auto hasFaces = detect();
#if 1
		if (hasFaces) {

			// ======== 检测到脸 ========
			// 设置起始帧计数
			if (flagAcceptNewFace) {
				
			  // ==== 新脸出现 ====
				// 回溯追寻确切的消失帧 (找到没有脸的那一帧为止)
				//frameCount = relocateFrame_R2L(false, left - step + 1, left - 1);
				//std::cout << "relocate frame: " << left - 1 << " -> " << frameCount << std::endl;

				detectStartFrameCount = frameCount;
				detectStartMs = getFrameMSec();
				flagAcceptNewFace = false;
			}

			// 设置最近一次帧计数
			lastFrameCount = frameCount;
			lastMs = getFrameMSec();

			//std::cout << "faces at: " << timestamp << ", frame=" 
			// << frameCount << ", face_num=" << faces.size() << std::endl;

		} else { // 没有帧当前帧检测到人脸，即人脸在当前帧刚刚消失或者已经消失了多个帧
			if (flagAcceptNewFace) // 要遇到的是新的脸
				goto CONT;

			// ==== 旧脸消失 ====
			// 回溯追寻确切的消失帧 (找到有脸的那一帧为止)
			//frameCount = relocateFrame_R2L(true, left - step + 1, left - 1);
			//std::cout << "relocate frame: " << left - 1 << " -> " << frameCount << std::endl;
			
			// 切断判断
			if (frameCount - lastFrameCount >= static_cast<size_t>(_cutcfg.cutThs)) {
				// 接下来，判断持续时间
				if (lastFrameCount - detectStartFrameCount >= static_cast<size_t>(_cutcfg.ignThs)) {
					// 持续时间够长，不忽略
					auto left = _cutcfg.preComp > static_cast<size_t>(detectStartFrameCount)
						? 0 : (detectStartFrameCount - static_cast<size_t>(_cutcfg.preComp));
					auto right = lastFrameCount + static_cast<size_t>(_cutcfg.postComp);

                    std::cout << "cut: " << left << " -> " << right << " ; "
						<< Utils::msToHMS(detectStartMs) << " -> " 
						<< Utils::msToHMS(lastMs) << std::endl;

					//lw << Utils::msToHMS(detectStartMs) << " " << Utils::msToHMS(lastMs) << "\n";
                    lw << ListEntry{detectStartFrameCount, lastFrameCount, Utils::msToHMS(detectStartMs),
                                    Utils::msToHMS(lastMs)};


				} // 忽略噪声帧
				flagAcceptNewFace = true;

			} else {
				// 继续下一帧，再探人脸是否复现
			}
		}
#endif

		CONT:
		if (frameCount % 10000 == 0) {
			std::cout << "thread " << std::this_thread::get_id() << ", reach frame: " << frameCount << std::endl;
		}

		left += step;
	}

    if (flagAcceptNewFace) // 要遇到的是新的脸
        return;

    {
        // 切断判断
        //if (frameCount - lastFrameCount >= static_cast<size_t>(_cutcfg.cutThs)) {
            // 接下来，判断持续时间
            //if (lastFrameCount - detectStartFrameCount >= static_cast<size_t>(_cutcfg.ignThs)) {
                // 持续时间够长，不忽略
                auto left = _cutcfg.preComp > static_cast<size_t>(detectStartFrameCount)
                            ? 0 : (detectStartFrameCount - static_cast<size_t>(_cutcfg.preComp));
                auto right = lastFrameCount + static_cast<size_t>(_cutcfg.postComp);

                std::cout << "cut: " << left << " -> " << right << " ; "
                          << Utils::msToHMS(detectStartMs) << " -> "
                          << Utils::msToHMS(lastMs) << std::endl;

                //lw << Utils::msToHMS(detectStartMs) << " " << Utils::msToHMS(lastMs) << "\n";
                lw << ListEntry{detectStartFrameCount, lastFrameCount, Utils::msToHMS(detectStartMs),
                                Utils::msToHMS(lastMs)};

            //} // 忽略噪声帧
            flagAcceptNewFace = true;
        //}
    }
}

void FaceDetector::setFramePos(size_t pos) {
    _detector.videoCapture()->set(cv::CAP_PROP_POS_FRAMES, pos);
}

size_t FaceDetector::getFramePos() {
    return _detector.videoCapture()->get(cv::CAP_PROP_POS_FRAMES);
}

double FaceDetector::getFrameMSec() {
    return _detector.videoCapture()->get(cv::CAP_PROP_POS_MSEC);
}

double FaceDetector::getFPS() {
    return _detector.videoCapture()->get(cv::CAP_PROP_FPS);
}