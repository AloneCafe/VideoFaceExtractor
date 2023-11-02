
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

	po::options_description desc("========== VideoFaceExtractor [视频人脸检测提取器] ========== \n* 可用选项");
	desc.add_options()
		("help,h", "输出帮助信息")
		("use-sec,s", "使用秒为单位时间，而不是帧 (默认设置)...")
		("pre-comp,m", po::value<double>()->value_name(" N "), "前置补偿：该人脸出现之前，额外在切分的视频中补偿前面的 N 个单位时间的影像")
		("post-comp,n", po::value<double>()->value_name(" N "), "后置补偿：该人脸消失之后，额外在切分的视频中补偿后面的 N 个单位时间的影像")
		("ign-ths,i", po::value<double>()->value_name(" N " ), "忽略阈值：当人脸持续时间少于 N 个单位时间，则忽略它")
		("cut-ths,c", po::value<double>()->value_name(" N "), "切断阈值：当后续 N 个单位时间不再存在人脸，则进行切分")
        ("ffmpeg,f", po::value<std::string>()->value_name("path"), "FFMPEG 核心程序：默认为 /bin/ffmpeg")
		("outlst,l", po::value<std::string>()->value_name("path"), "输出清单文件：识别结果将存储在这个清单文件中，以供给切片程序使用")
        ("outflt,L", po::value<std::string>()->value_name("path"), "输出Filter文件：识别结果将存储在这个Filter文件中，以供给切片程序使用 (性能更好)")
        ("outfile,o", po::value<std::string>()->value_name("path"), "输出视频切片文件的目录")
        ("only-detect", "仅执行检测，而不切分 (仅检测人脸并生成时间清单)")
        ("only-split", "仅执行切分，而不检测 (仅根据已生成的时间清单切分视频)")
        ("extra-options", po::value(&extra), "输入文件");
	po::positional_options_description p;
	p.add("extra-options", -1);

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

	if (vm.count("use-sec")) {
		cutcfg.useFrame = false;
	} else {
		;
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
    if (vm.count("outfile")) {
        outFile = vm["outfile"].as<std::string>();
    } else {
        std::cerr << "错误，没有指定输出视频文件！" << std::endl;
        return 1;
    }

	if (vm.count("outlst")) {
		outLst = vm["outlst"].as<std::string>();
	} else {
		std::cout << "警告，没有指定输出的清单文件路径！\n程序将会把结果打印到标准输出" << std::endl;
	}
    if (vm.count("outflt")) {
        outFlt = vm["outflt"].as<std::string>();
    } else {
        std::cerr << "警告，没有指定输出的 FFmpeg Filter 文件路径！\n" << std::endl;
        return 1;
    }

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