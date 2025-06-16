-- 设置项目名称和版本
set_project("MetasequoiaImeTsf")
set_version("1.0.0", { build = "%Y%m%d%H%M" })

set_xmakever("3.0.0")
if is_arch("x64") then
    add_defines("_X64_")
    add_defines("WINVER=0x0A00")
    add_defines("NTDDI_VERSION=0x0A000000")
end

-- 设置目标语言和标准
set_languages("c++17")
set_toolchains("msvc")

-- 添加定义
add_defines("_UNICODE=1", "UNICODE=1")

-- 添加编译选项
add_cxflags("/Zc:__cplusplus")

add_requires("fmt")
add_requires("utfcpp")

add_syslinks({
    "ole32",
    "oleaut32",
    "user32",
    "gdi32",
    "advapi32",
    "comctl32",
    "shcore",
    "imm32",
    "uxtheme",
    "dwmapi"
})

-- 添加包含目录
add_includedirs("./src", { public = true })
add_includedirs("./src/Candidate", { public = true })
add_includedirs("./src/Compartment", { public = true })
add_includedirs("./src/Composition", { public = true })
add_includedirs("./src/DictEngine", { public = true })
add_includedirs("./src/DisplayAttribute", { public = true })
add_includedirs("./src/Drawing", { public = true })
add_includedirs("./src/Edit", { public = true })
add_includedirs("./src/FanyLog", { public = true })
add_includedirs("./src/File", { public = true })
add_includedirs("./src/Global", { public = true })
add_includedirs("./src/Header", { public = true })
add_includedirs("./src/IME", { public = true })
add_includedirs("./src/IPC", { public = true })
add_includedirs("./src/Key", { public = true })
add_includedirs("./src/LanguageBar", { public = true })
add_includedirs("./src/Register", { public = true })
add_includedirs("./src/Tf", { public = true })
add_includedirs("./src/Thread", { public = true })
add_includedirs("./src/UI", { public = true })
add_includedirs("./src/Utils", { public = true })
add_includedirs("./src/Window", { public = true })

-- 定义目标
target("MetasequoiaImeTsf")
set_kind("shared") -- 设置为目标类型为共享库
add_files("./src/Candidate/EnumTfCandidates.cpp",
    "./src/Candidate/SearchCandidateProvider.cpp",
    "./src/Candidate/TipCandidateList.cpp",
    "./src/Candidate/TipCandidateString.cpp",
    "./src/Compartment/Compartment.cpp",
    "./src/Composition/Composition.cpp",
    "./src/Composition/CompositionProcessorEngine.cpp",
    "./src/Composition/EndComposition.cpp",
    "./src/Composition/StartComposition.cpp",
    "./src/DictEngine/BaseDictionaryEngine.cpp",
    "./src/DictEngine/DictionaryParser.cpp",
    "./src/DictEngine/DictionarySearch.cpp",
    "./src/DictEngine/TableDictionaryEngine.cpp",
    "./src/DisplayAttribute/DisplayAttribute.cpp",
    "./src/DisplayAttribute/DisplayAttributeInfo.cpp",
    "./src/DisplayAttribute/DisplayAttributeProvider.cpp",
    "./src/Drawing/D2DSource.cpp",
    "./src/Edit/EditSession.cpp",
    "./src/Edit/GetTextExtentEditSession.cpp",
    "./src/Edit/TextEditSink.cpp",
    "./src/File/File.cpp",
    "./src/File/FileMapping.cpp",
    "./src/Global/Globals.cpp",
    "./src/IME/ActiveLanguageProfileNotifySink.cpp",
    "./src/IME/FunctionProviderSink.cpp",
    "./src/IME/MetasequoiaIME.cpp",
    "./src/IME/MetasequoiaIME.rc", -- 资源文件
    "./src/IME/MetasequoiaIMEBaseStructure.cpp",
    "./src/IPC/Ipc.cpp",
    "./src/Key/KeyEventSink.cpp",
    "./src/Key/KeyHandler.cpp",
    "./src/Key/KeyHandlerEditSession.cpp",
    "./src/Key/KeyStateCategory.cpp",
    "./src/LanguageBar/LanguageBar.cpp",
    "./src/Register/Register.cpp",
    "./src/Register/RegKey.cpp",
    "./src/Tf/TfInputProcessorProfile.cpp",
    "./src/Tf/TfTextLayoutSink.cpp",
    "./src/Thread/ThreadFocusSink.cpp",
    "./src/Thread/ThreadMgrEventSink.cpp",
    "./src/UI/CandidateListUIPresenter.cpp",
    "./src/DisplayAttribute/EnumDisplayAttributeInfo.cpp",
    "./src/Utils/CommonUtils.cpp",
    "./src/Utils/FanyUtils.cpp",
    "./src/Window/BaseWindow.cpp",
    "./src/Window/ButtonWindow.cpp",
    "./src/Window/CandidateWindow.cpp",
    "./src/Window/ScrollBarWindow.cpp",
    "./src/Window/ShadowWindow.cpp",
    "./src/DllMain.cpp",
    "./src/Server.cpp"
)

add_files("./src/IME/MetasequoiaIME.def") -- 添加模块定义文件
add_packages("fmt")
add_packages("utfcpp")
