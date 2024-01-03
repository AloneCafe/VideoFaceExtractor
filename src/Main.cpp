
#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <iostream>

#include "App.h"


namespace po = boost::program_options;


int main(int ac, char** av) try {

#if defined(WIN32) || defined(WIN64)
    system("chcp 65001");
#endif

	std::vector<std::string> extra;

	po::options_description desc("------------------------- VideoFaceExtractor [视频人脸检测提取器] ------------------------- \n* 可用选项", 500);
	desc.add_options()
		("help,h", "输出帮助信息\n")

        ("-------------------------", "[以下是视频文件模式]    -------------------------\n")

        ("infile,i", po::value<std::string>()->value_name("path"),
             "输入视频文件\n")

//        ("indir,I", po::value<std::string>()->value_name("path"),
//         "输入视频目录\n")

		("pre-comp,m", po::value<double>()->value_name(" N "),
                "前置补偿：该人脸出现之前，额外在切分的视频中补偿前面的 N 个单位时间的影像\n")
		("post-comp,n", po::value<double>()->value_name(" N "),
                "后置补偿：该人脸消失之后，额外在切分的视频中补偿后面的 N 个单位时间的影像\n")
		("ign-ths", po::value<double>()->value_name(" N " ),
                "忽略阈值：当人脸持续时间少于 N 个单位时间，则忽略它\n")
		("cut-ths", po::value<double>()->value_name(" N "),
                "切断阈值：当后续 N 个单位时间不再存在人脸，则进行切分\n")

        ("ffmpeg,f", po::value<std::string>()->value_name("path"),
                "FFMPEG 核心程序：默认为 /bin/ffmpeg 或者 ffmpeg.exe (in %PATH%) \n")
		("outlst,l", po::value<std::string>()->value_name("path"),
                "输出清单文件：识别结果将存储在这个清单文件中，以供给切片程序使用\n")
        ("outflt,L", po::value<std::string>()->value_name("path"),
                "输出Filter文件：识别结果将存储在这个Filter文件中，以供给切片程序使用 (性能更好)\n")


        ("only-detect", "仅执行检测，而不切分 (仅检测人脸并生成时间清单)\n")
        ("only-split", "仅执行切分，而不检测 (仅根据已生成的时间清单切分视频)\n")

            ("-------------------------", "[以下是实时监控模式]    -------------------------\n")

        ("from-camera,c", po::value<std::string>()->value_name("caps-numbers"),
                "输入系统摄像头设备编号列表，以逗号分隔\n"
                "一旦程序工作在该模式下，将自动忽略四个参数 N，\n"
                "忽略一切有关 ffmpeg 的参数（包括输出的清单文件与Filter文件），\n"
                "并且将执行全过程的检测与切分操作，\n只有时间单位设置（秒/帧）、输出视频切片文件参数是有效的\n"
                "（而不能单独运行检测或者切分的其中一项）\n")
        ("camera-nogui", "在系统的实时摄像头捕获（在线监控）模式下，不显示监控 GUI 图形界面\n")

        ("use-sec,s", "使用秒为单位时间，而不是帧 (默认设置)...\n")

        ("outfile,o", po::value<std::string>()->value_name("path"),
            "输出视频切片文件\n");

        //("extra-options", po::value(&extra), "输入文件\n");
	po::positional_options_description p;
	//p.add("extra-options", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || ac < 2) {
		std::cout << desc << "\n";
		return ac < 2 ? 1 : 0;
	}

	CutConfig cutcfg{};
	cutcfg.useFrame = true;
	cutcfg.postComp = 0;
	cutcfg.preComp = 0;
	cutcfg.ignThs = 30;
	cutcfg.cutThs = 30;

	std::string outFile, outLst, outFlt, inFile;
    std::string ffmpegPath;

    // 参数解析
    if (vm.count("infile")) {
        inFile = vm["infile"].as<std::string>();
    } else {
        //std::cerr << "错误，没有指定输入视频文件！" << std::endl;
        //return 1;
    }

    std::vector<int> cameras;
    if (vm.count("from-camera")) {
        std::string s = vm["from-camera"].as<std::string>();
        const char *sep = ",";
        const char *p = strtok(s.data(), sep);
        while (p) {
            char *endptr;
            auto i = strtol(p, &endptr, 10);
            if (errno != 0) {
                std::cerr << "错误，非法的系统摄像头编号！" << std::endl;
                return 1;
            }
            cameras.push_back( i );
            p = strtok(NULL, sep);
        }

    } else {
        //std::cerr << "错误，没有指定输入摄像头！" << std::endl;
        //return 1;
    }

    if (not inFile.empty() && not cameras.empty()) {
        std::cerr << "无法同时指定两种不同的输入源（同时指定了系统摄像头与视频文件）！" << std::endl;
        return 1;
    }
    if (inFile.empty() && cameras.empty()) {
        std::cerr << "错误，没有指定任何视频源！" << std::endl;
        return 1;
    }

	if (vm.count("use-sec")) {
		cutcfg.useFrame = false;
	} else {
		;
	}

    if (vm.count("outfile")) {
        outFile = vm["outfile"].as<std::string>();
    } else {
        std::cerr << "错误，没有指定输出视频文件！" << std::endl;
        return 1;
    }

    // 判定输入是否为系统摄像头
    if (not cameras.empty()) {
        return App{}.full_run(cameras, cutcfg, /*outDir,*/ outLst, outFlt, ffmpegPath, outFile);
    }

	if (vm.count("post-comp")) {
		cutcfg.postComp = vm["post-comp"].as<double>();
	} else {
		;
	}
	if (vm.count("pre-comp")) {
		cutcfg.preComp = vm["pre-comp"].as<double>();
	} else {
		;
	}
	if (vm.count("ign-ths")) {
		cutcfg.ignThs = vm["ign-ths"].as<double>();
	} else {
		;
	}
	if (vm.count("cut-ths")) {
		cutcfg.cutThs = vm["cut-ths"].as<double>();
	} else {
		;
	}
    if (vm.count("ffmpeg")) {
        ffmpegPath = vm["ffmpeg"].as<std::string>();
    } else {
        ;
    }


	if (vm.count("outlst")) {
		outLst = vm["outlst"].as<std::string>();
	} else {
		std::cout << "警告，没有指定输出的清单文件路径！\n程序将会把结果打印到标准输出" << std::endl;
	}
    if (vm.count("outflt")) {
        outFlt = vm["outflt"].as<std::string>();
    } else {
        std::cerr << "错误，没有指定输出的 FFmpeg Filter 文件路径！\n" << std::endl;
        return 1;
    }

#if 0
	if (extra.empty()) {
		std::cerr << "错误，没有指定输入视频文件！" << std::endl;
		return 1;
	} else if (extra.size() > 1) {
		std::cerr << "错误，只允许输入一个视频文件！" << std::endl;
		return 1;
	} else {
		inFile = std::move(extra.back());
		extra.pop_back();
	}
#endif





    bool
      onlyDetect = vm.count("only-detect"),
      onlySplit = vm.count("only-split");

    if (onlySplit && onlyDetect) {
        std::cerr << "错误，不能同时指定 --only-detect 和 --only-split ！" << std::endl;
        return 1;
    }


    if (onlyDetect) {
        return App{}.only_detect_run(inFile, cutcfg, outLst, outFlt);
    } else if (onlySplit) {
        return App{}.only_split_run(inFile, outLst, outFlt, ffmpegPath, outFile);
    } else {
        return App{}.full_run(inFile, cutcfg, /*outDir,*/ outLst, outFlt, ffmpegPath, outFile);
    }


} catch (std::exception& e) {
	std::cerr << "错误: " << e.what() << "\n";
	return 1;
} catch (...) {
	std::cerr << "程序运行错误" << std::endl;
	return 1;
}