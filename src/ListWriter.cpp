
#include <atomic>
#include "ListWriter.h"

std::ofstream ListWriter::_olist;

std::ofstream ListWriter::_ofilt;

std::mutex ListWriter::_mtx;

std::priority_queue<ListEntry> ListWriter::_pq;

std::atomic<int> ListWriter::_ref_count = 0;