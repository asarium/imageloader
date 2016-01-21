#pragma once

#include <imageloader.h>

#include <exception>

namespace imgload
{
    class Exception : public std::exception
    {
        ImgloadErrorCode m_err;

    public:
        explicit Exception(ImgloadErrorCode m_err) : m_err(m_err)
        { }

        virtual const char* what() const noexcept;
    };
}


