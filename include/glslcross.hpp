//
// Created by selya on 31.08.2019.
//

#ifndef GLSLCROSS_GLSLCROSS_HPP
#define GLSLCROSS_GLSLCROSS_HPP

#include <map>
#include <vector>
#include <string>

namespace glslcross {

enum class Stage {
    Vertex,
    Fragment,
    Geometry
};

std::map<Stage, std::vector<uint32_t>> SourceToSpirv(const std::map<Stage, std::string> &sources);

std::map<Stage, std::string> SpirvToSource(
        const std::map<Stage, std::vector<uint32_t>> &spirv, int version, bool es = false);

}

#endif //GLSLCROSS_GLSLCROSS_HPP
