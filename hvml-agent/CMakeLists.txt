# Make sure ACE_ROOT was set and necesssary files were already built
if (NOT DEFINED ENV{ACE_ROOT})
    message(FATAL_ERROR "ACE_ROOT not set. Set to the root diretcory of ACE, which must be already be built sucessfully.")
endif()

set(hvml_agent_srcs         
    hvml-agent.cpp
    HttpServer.cpp
    HvmlEcho.cpp
    JsonQuery.cpp
    Evaluator.cpp
    HvmlRuntime.cpp
)


add_executable(hvml-agent ${hvml_agent_srcs})

target_include_directories(hvml-agent PUBLIC
    ../include
    $ENV{ACE_ROOT}
    $ENV{ACE_ROOT}/apps/JAWS2
)

target_link_directories(hvml-agent PUBLIC $ENV{ACE_ROOT}/lib)

target_link_libraries(hvml-agent
    hvml_interpreter
    hvml_parser
    HTTPU
    ACE
    )