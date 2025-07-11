#pragma once

// copy from grpc

template <typename CppType, typename CType>
class CppImplOf {
 public:
  // Convert the C struct to C++
  static CppType* FromC(CType* c_type) {
    return reinterpret_cast<CppType*>(c_type);
  }

  static const CppType* FromC(const CType* c_type) {
    return reinterpret_cast<const CppType*>(c_type);
  }

  // Retrieve a c pointer (of the same ownership as this)
  CType* c_ptr() {
    return reinterpret_cast<CType*>(static_cast<CppType*>(this));
  }

 protected:
  ~CppImplOf() = default;
};