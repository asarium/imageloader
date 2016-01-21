//
// Created by marius on 1/12/16.
//

#include "Exception.hpp"

const char* imgload::Exception::what() const noexcept
{
    switch(m_err)
    {
        case IMGLOAD_ERR_NO_ERROR:
            return "No error has occured (why are you seeing this?)";
        case IMGLOAD_ERR_OUT_OF_MEMORY:
            return "A memory allocation has failed";
        case IMGLOAD_ERR_PLUGIN_INVALID:
            return "A plugin wasn't initialized correctly";
        case IMGLOAD_ERR_UNSUPPORTED_FORMAT:
            return "The file format is not supported";
        case IMGLOAD_ERR_PLUGIN_ERROR:
            return "A generic plugin error has occurred";
        case IMGLOAD_ERR_IO_ERROR:
            return "An IO error has occured";
        case IMGLOAD_ERR_OUT_OF_RANGE:
            return "A value was out of range";
        case IMGLOAD_ERR_NO_DATA:
            return "There was no data for the given parameters";
        case IMGLOAD_ERR_WRONG_TYPE:
            return "The type of the property did not match";
        case IMGLOAD_ERR_FILE_INVALID:
            return "The file is not valid";
        case IMGLOAD_ERR_UNSUPPORTED_CONVERSION:
            return "The requested conversion is not supported";
        default:
            return "Unknown error";
    }
}
