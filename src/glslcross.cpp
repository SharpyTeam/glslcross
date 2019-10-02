//
// Created by selya on 31.08.2019.
//

#include <glslcross.hpp>

#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
#include <SPIRV/GlslangToSpv.h>
#endif
#include <glslang/Public/ShaderLang.h>
#include <spirv_glsl.hpp>

#include <stdexcept>

namespace glslcross {

static EShLanguage StageToEShLanguage(Stage stage) {
    switch (stage) {
        case Stage::Vertex: return EShLanguage::EShLangVertex;
        case Stage::Fragment: return EShLanguage::EShLangFragment;
        case Stage::Geometry: return EShLanguage::EShLangGeometry;
    }
    throw std::runtime_error("Unsupported stage");
}

#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
static std::pair<bool, std::shared_ptr<glslang::TShader>> CompileShader(const std::string &text, EShLanguage type) {
    auto shader = std::make_shared<glslang::TShader>(type);
    shader->setAutoMapLocations(true);
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

std::map<Stage, std::vector<uint32_t>> SourceToSpirv(const std::map<Stage, std::string> &sources) {
#if GLSLCROSS_GLSL_TO_SPIRV_COMPILER
    glslang::InitializeProcess();

    glslang::TProgram program;
    std::map<Stage, std::shared_ptr<glslang::TShader>> tshaders;
    for (auto &s : sources) {
        auto result = CompileShader(s.second, StageToEShLanguage(s.first));
        tshaders[s.first] = result.second;
        if (!result.first) {
            glslang::FinalizeProcess();
            throw std::runtime_error("Can't compile shader:\n" + std::string(result.second->getInfoLog()));
        }
        program.addShader(result.second.get());
    }

    if (!program.link(EShMessages::EShMsgDefault)) {
        glslang::FinalizeProcess();
        throw std::runtime_error("Can't link shader:\n" + std::string(program.getInfoLog()));
    }

    std::map<Stage, std::vector<uint32_t>> result;
    for (auto &s : sources) {
        auto intermediate = program.getIntermediate(StageToEShLanguage(s.first));
        glslang::GlslangToSpv(*intermediate, result[s.first]);
    }

    glslang::FinalizeProcess();

    return result;
#else
    static_assert(false, "GLSL to SPIRV compilation disabled (define GLSLCROSS_GLSL_TO_SPIRV_COMPILER)");
#endif
}

std::map<Stage, std::string>SpirvToSource(
        const std::map<Stage, std::vector<uint32_t>> &spirv, int version, bool es) {

    std::map<Stage, std::string> result;

    for (auto &s : spirv) {
        spirv_cross::CompilerGLSL glsl(s.second);
        spirv_cross::CompilerGLSL::Options options;
        options.version = version;
        options.es = es;
        glsl.set_common_options(options);
        result[s.first] = glsl.compile();
    }

    return result;
}

}