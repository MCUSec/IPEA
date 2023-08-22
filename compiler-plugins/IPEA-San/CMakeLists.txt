add_llvm_library(ipea-san MODULE BUILDTREE_ONLY
    # List your source files here.
    IPEASan.cpp

    DEPENDS
    intrinsics_gen
    PLUGIN_TOOL
    opt
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_compile_options(ipea-san PRIVATE -g -Og)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
#target_compile_features(SkeletonPass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that.
#set_target_properties(SkeletonPass PROPERTIES
#    COMPILE_FLAGS "-fno-rtti"
#)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(ipea-san PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
