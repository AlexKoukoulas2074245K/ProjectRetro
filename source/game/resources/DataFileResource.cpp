//
//  DataFileResource.cpp
//  Hardcore2D
//
//  Created by Alex Koukoulas on 14/01/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "DataFileResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

DataFileResource::DataFileResource(const std::string& contents)
    : mContents(contents)
{
    
}

const std::string& DataFileResource::GetContents() const
{
    return mContents;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////