// Copyright (C) 2011  Lukas Lalinsky
// Distributed under the MIT license, see the LICENSE file for details.

#include "segment_merger.h"

using namespace Acoustid;

SegmentMerger::SegmentMerger(SegmentDataWriter *writer)
	: m_writer(writer)
{
}

SegmentMerger::~SegmentMerger()
{
	qDeleteAll(m_readers);
	delete m_writer;
}

size_t SegmentMerger::merge()
{
	QList<SegmentEnum *> readers(m_readers);
	QMutableListIterator<SegmentEnum *> iter(readers);
	while (iter.hasNext()) {
		SegmentEnum *reader = iter.next();
		if (!reader->next()) {
			iter.remove();
		}
	}
	QSet<int> docs;
	uint64_t lastMinItem = ~0;
	while (!readers.isEmpty()) {
		size_t minItemIndex = 0;
		uint64_t minItem = ~0;
		for (size_t i = 0; i < readers.size(); i++) {
			SegmentEnum *reader = readers[i];
			uint64_t item = (uint64_t(reader->key()) << 32) | reader->value();
			if (item < minItem) {
				minItem = item;
				minItemIndex = i;
			}
		}
		if (!readers[minItemIndex]->next()) {
			readers.removeAt(minItemIndex);
		}
		if (minItem != lastMinItem) {
			uint32_t key = (minItem >> 32) & 0xffffffff;
			uint32_t value = minItem & 0xffffffff;
			docs.insert(value);
			m_writer->addItem(key, value);
			lastMinItem = minItem;
		}
	}
	m_writer->close();
	return docs.size();
}
