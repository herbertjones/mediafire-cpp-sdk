TODO: Update documentation after JSON conversion

api: The API path
version: Namespace name for version.
session_token: Default is use session token.  Use NoSessionToken if session
    token is not expected for this type.
system_cpp_includes, local_cpp_includes, system_hpp_includes, local_hpp_includes:
    Additional include files to be added.
enums: C++ enumerated type
    1: List of tuples:
    1: C++ type name
    2: List of tuples with C++ <-> REST translations. Goes both ways.
        1: C++ enum value
        2: REST value
return_structs: C++ structs for the return type.
    1: List of tuples
        1: Struct type name
        2: List same as arguments of return_params
input_params: Required input data
    1: Tuple with required input parameters.
        1: C++ type (May be previously defined enum)
        2: C++ variable name
        3: REST name
optional_input_params: Optional parameters only sent if set
    1: List of tuples
        1: C++ type (May be previously defined enum)
        2: C++ variable name
        3: REST name
return_params: Expected return data
    1: List of tuples
        1: C++ type (May be previously defined enum or struct)
        2: C++ variable name
        3: JSON path separated by dots
        4: JSON expected type, consisting of:
            TSingle: A normal variable from JSON.
            TArray: The variable is an array in JSON. The C++ type will be
                    wrapped in a std::vector.
            TArrayFront: An array is returned in JSON, but we only care about
                         the first entry.
        5: Optional setting
            Required: The response is expected and required.
            Optional: The response is not required. It will be wrapped in
                      boost::optional if a default is not given. If a default is
                      given, the default value will be set if no value parsed.

When conversion to string from type that doesn't have a mf::utils::to_string
override, you may provide an AsString override in type_helpers.[ch]pp.
