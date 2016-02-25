#ifndef _INDIRECT_REFERENCE_TABLE_H_
#define _INDIRECT_REFERENCE_TABLE_H_


#include <cstdint>


// A mirror of the Runtime managed object reference management table.
struct IndirectReferenceTable
{
    jobject Add(uint32_t, void*);
    bool Remove(uint32_t, jobject);
};


#endif