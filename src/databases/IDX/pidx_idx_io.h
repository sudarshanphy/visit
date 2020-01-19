/***************************************************
 ** ViSUS Visualization Project                    **
 ** Copyright (c) 2010 University of Utah          **
 ** Scientific Computing and Imaging Institute     **
 ** 72 S Central Campus Drive, Room 3750           **
 ** Salt Lake City, UT 84112                       **
 **                                                **
 ** For information about this project see:        **
 ** http://www.pascucci.org/visus/                 **
 **                                                **
 **      or contact: pascucci@sci.utah.edu         **
 **                                                **
 ****************************************************/

#ifndef _pidx_idx_io_h
#define _pidx_idx_io_h

#include <string>
#include <vector>
#include <cassert>

#include <PIDX.h>

#include "visit_idx_io.h"
#include "visit_idx_io_types.h"

typedef std::string String;

class PIDXIO : public IDX_IO{

public:
    
    PIDXIO(){}
  
    bool openDataset(const std::string filename);
    
    unsigned char* getData(const VisitIDXIO::Box box, const int timestate, const char* varname);
    unsigned char* getParticleData(const VisitIDXIO::Box box, const int timestate, const char* varname);
    uint64_t getParticleDataCount();
    
    std::vector<int> getGlobalSize();

    PIDX_variable *checkpoint_vars = NULL;
    uint64_t *checkpoint_particle_counts = NULL;
    void **checkpoint_data = NULL;

    virtual ~PIDXIO();

};


#endif
