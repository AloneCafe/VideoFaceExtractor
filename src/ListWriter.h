#pragma once
#include <fstream>
#include <mutex>
#include <queue>
#include <utility>

struct ListEntry {
    size_t startPos;
    size_t endPos;
    std::string startTime;
    std::string endTime;
    ListEntry(size_t startPos, size_t endPos, std::string startTime, std::string endTime)
       : startPos(startPos), endPos(endPos), startTime(std::move(startTime)), endTime(std::move(endTime)) {}
    ~ListEntry() = default;
    bool operator<(const ListEntry & rhs) const {
        return this->startPos > rhs.startPos;
    }
};

struct ListWriter {
private:
	static std::ofstream _olist;
    static std::ofstream _ofilt;
	static std::mutex _mtx;
    static std::priority_queue<ListEntry> _pq;
    static std::atomic<int> _ref_count;

public:
	static bool globalOpen(const std::string& path_list, const std::string& path_filt) {
		_olist.open(path_list, std::ios::out);
        _ofilt.open(path_filt, std::ios::out);
        return _olist.is_open() && _ofilt.is_open();
	}

	static void globalClose() {
        //writeToLstFile();
        _olist.close();
        writeToFilterFile();
	}

private:
    static void writeToFilterFile() {
        size_t total = 0;
        while (!_pq.empty()) {
            auto le = _pq.top();
            _ofilt << "[0:v]trim=start_frame=" << le.startPos << ":end_frame=" << le.endPos << ",setpts=PTS-STARTPTS[v" << total++ << "];\n";
            _pq.pop();
        }

        for (size_t i = 0; i < total; ++i) {
            _ofilt << "[v" << i << "]";
        }

        _ofilt << "concat=n=" << total << ":v=1:a=0[v]\n";
        _ofilt.close();
    }

//    static void writeToLstFile() {
//        while (!_pq.empty()) {
//            auto le = _pq.top();
//            _olist << le.startTime << " " << le.endTime << "\n";
//            _pq.pop();
//        }
//        _olist.close();
//    }

public:
    ~ListWriter() {
        if (--_ref_count <= 0) {
            globalClose();
        }
    }

    ListWriter() {
        ++_ref_count;
    }

    ListWriter& operator<<(const ListEntry & arg) {
        {
            std::lock_guard<std::mutex> lck(_mtx);
            _pq.push(arg);
            _olist << arg.startTime << " " << arg.endTime << "\n";
        }
        return *this;
    }
};

//template <typename Arg>
//ListWriter& ListWriter::operator<<(Arg&& arg)
//{
//	{
//		std::lock_guard<std::mutex> lck(_mtx);
//		_olist << arg;
//		_olist.flush();
//	}
//	return *this;
//}

