/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2015, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GRAPHICS_OPENGL_GEOMETRYBUFFER_HPP_
#define GRAPHICS_OPENGL_GEOMETRYBUFFER_HPP_

#include <Graphics/Buffers/GeometryBuffer.hpp>

namespace sgl {

class DLL_OBJECT GeometryBufferGL : public GeometryBuffer
{
public:
    GeometryBufferGL(size_t size, BufferType type = VERTEX_BUFFER, BufferUse bufferUse = BUFFER_STATIC);
    GeometryBufferGL(size_t size, void *data, BufferType type = VERTEX_BUFFER, BufferUse bufferUse = BUFFER_STATIC);
    ~GeometryBufferGL();
    void subData(int offset, size_t size, void *data);
    void *mapBuffer(BufferMapping accessType);
    void *mapBufferRange(int offset, size_t size, BufferMapping accessType);
    void unmapBuffer();
    void bind();
    void unbind();
    inline unsigned int getBuffer() { return buffer; }
    inline unsigned int getGLBufferType() { return oglBufferType; }

private:
    void initialize(BufferType type, BufferUse bufferUse);
    unsigned int buffer;
    unsigned int oglBufferType;
    unsigned int oglBufferUsage;
};

}

/*! GRAPHICS_OPENGL_GEOMETRYBUFFER_HPP_ */
#endif
