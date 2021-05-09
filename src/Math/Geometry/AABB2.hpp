/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2017, Christoph Neuhauser
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

#ifndef SRC_MATH_GEOMETRY_AABB2_HPP_
#define SRC_MATH_GEOMETRY_AABB2_HPP_

#include <glm/glm.hpp>
#include <cfloat>
#include <Defs.hpp>

namespace sgl {

class DLL_OBJECT AABB2 {
public:
    glm::vec2 min, max;

    AABB2() : min(FLT_MAX), max(-FLT_MAX) {}
    AABB2(glm::vec2 min, glm::vec2 max) : min(min), max(max) {}

    inline glm::vec2 getDimensions() const { return max - min; }
    inline glm::vec2 getExtent() const { return (max - min) / 2.0f; }
    inline glm::vec2 getCenter() const { return (max + min) / 2.0f; }
    inline glm::vec2 getMinimum() const { return min; }
    inline glm::vec2 getMaximum() const { return max; }
    inline float getWidth() const { return max.x - min.x; }
    inline float getHeight() const { return max.y - min.y; }

    //! Merge the two AABBs
    void combine(const AABB2 &otherAABB);
    //! Merge AABB with a point
    void combine(const glm::vec2 &pt);
    //! Transform AABB
    AABB2 transformed(const glm::mat4 &matrix);
};

}

/*! SRC_MATH_GEOMETRY_AABB2_HPP_ */
#endif
