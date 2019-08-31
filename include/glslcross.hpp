//
// Created by selya on 31.08.2019.
//

#ifndef GLSLCROSS_GLSLCROSS_HPP
#define GLSLCROSS_GLSLCROSS_HPP

#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
#include <SPIRV/GlslangToSpv.h>
#endif
#include <glslang/Public/ShaderLang.h>
#include <spirv_glsl.hpp>

#include <iostream>
#include <memory>
#include <map>

namespace glslcross {

class ShaderProgram {
public:
    enum class Stage {
        Vertex = EShLanguage::EShLangVertex,
        Fragment = EShLanguage::EShLangFragment,
        Geometry = EShLanguage::EShLangGeometry
    };

    class ShaderData {
        friend class ShaderProgram;

    public:
        const Stage stage;
        std::string source;
        std::vector<uint32_t> spirv;

        explicit ShaderData(Stage stage);

        const std::string &GetCrosscompiledSource();

    private:
        std::string crosscompiled_source;
    };

    ShaderProgram();

    ShaderData &GetShaderData(Stage stage);
    const std::string &GetInfoLog();
    bool Crosscompile(int target_version, bool es = false);

private:
    std::map<Stage, ShaderData> shaders;
    std::string info_log;
};

}

#endif //GLSLCROSS_GLSLCROSS_HPP
