set(LLVM_LINK_COMPONENTS core jit interpreter native)

add_llvm_example(Kaleidoscope
  toy.cpp
  )
