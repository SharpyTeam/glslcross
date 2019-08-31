//
// Created by selya on 31.08.2019.
//

#include <glslcross.hpp>

namespace glslcross {

#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
static std::pair<bool, std::shared_ptr<glslang::TShader>> CompileShader(const std::string &text, EShLanguage type) {
    auto shader = std::make_shared<glslang::TShader>(type);
    const char *c = text.c_str();
    shader->setStrings(&c, 1);
    shader->setEnvInput(glslang::EShSource::EShSourceGlsl, type, glslang::EShClient::EShClientOpenGL,
                        glslang::EshTargetClientVersion::EShTargetOpenGL_450);
    shader->setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EshTargetClientVersion::EShTargetOpenGL_450);
    shader->setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_4);
    TBuiltInResource t{};
    bool success = shader->parse(&t, 110, true, EShMessages::EShMsgDefault);
    return std::make_pair(success, shader);
}
#endif

ShaderProgram::ShaderData::ShaderData(ShaderProgram::Stage stage) : stage(stage) {}

const std::string &ShaderProgram::ShaderData::GetCrosscompiledSource() {
    return crosscompiled_source;
}

ShaderProgram::ShaderProgram() {}

ShaderProgram::ShaderData &ShaderProgram::GetShaderData(ShaderProgram::Stage stage) {
    if (shaders.find(stage) == shaders.end()) {
        shaders.emplace(stage, ShaderData(stage));
    }
    return shaders.at(stage);
}

const std::string &ShaderProgram::GetInfoLog() {
    return info_log;
}

bool ShaderProgram::Crosscompile(int target_version, bool es) {
    bool initialized = false;

#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
    glslang::TProgram program;
    info_log.clear();

    std::vector<std::shared_ptr<glslang::TShader>> ref_holder;
    for (auto &p : shaders) {
        ShaderData &data = p.second;
        if (data.spirv.empty()) {
            if (data.source.empty()) continue;
            if (!initialized) {
                glslang::InitializeProcess();
                initialized = true;
            }
            auto result = CompileShader(data.source, (EShLanguage) data.stage);
            ref_holder.push_back(result.second);
            if (!result.first) {
                glslang::FinalizeProcess();
                info_log = result.second->getInfoLog();
                return false;
            }
            program.addShader(result.second.get());
        }
    }

    if (initialized) {
        if (!program.link(EShMessages::EShMsgDefault)) {
            glslang::FinalizeProcess();
            info_log = program.getInfoLog();
            return false;
        }
    }
#endif

    for (auto &p : shaders) {
        ShaderData &data = p.second;
        if (data.spirv.empty()) {
#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
            auto intermediate = program.getIntermediate((EShLanguage) data.stage);
            data.spirv.clear();
            glslang::GlslangToSpv(*intermediate, data.spirv);
#else
            continue;
#endif
        }
        spirv_cross::CompilerGLSL glsl(data.spirv);
        spirv_cross::CompilerGLSL::Options options;
        options.version = target_version;
        options.es = es;
        glsl.set_common_options(options);
        data.crosscompiled_source = glsl.compile();
    }

#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
    if (initialized) {
        glslang::FinalizeProcess();
    }
#endif

    return true;
}


}