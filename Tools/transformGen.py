import os

NAME_BASE = "TransformFor"
GENERAL_NAME_BASE = "AllTransformsFor"


def get_description_comment():
    return "/*Auto Generated using transformGen.py, do not edit!*/\n\n"


def get_includes():
    includes = ["#include \"../common/aya-types.h\"", "#include <stdlib.h>", "#include <string.h>"]

    include_string = ""
    for i in includes:
        include_string = include_string + i + "\n"

    return include_string + "\n"


def get_all_scalar_types():
    scalar_types = ['i', 'u']
    bitness_vals = ['8', '16', '32', '64']
    all_types = ['f32', 'f64']

    for s in scalar_types:
        for b in bitness_vals:
            all_types.append(s + b)

    return all_types


def get_transform_operations():
    transform_operation_tuples = [("Sum", "+"), ("Sub", "-"), ("Mul", "*"), ("Div", "/"), ("Noop", "")]
    return transform_operation_tuples


def produce_function_name(transform_tuple, scalar_type):
    return "{}{}{}(void* base, void* transformer, void** storage, char** tag)".format(transform_tuple[0], NAME_BASE,
                                                                                      scalar_type)


def produce_general_function_name(scalar_type):
    return "Get{}{}(size_t* count)".format(GENERAL_NAME_BASE, scalar_type.upper())


def get_func_definition(transform_tuple, scalar_type):
    func_name = produce_function_name(transform_tuple, scalar_type)
    return "void {} {{\n\tstrcpy(*tag, \"{}\\0\");\n\tif(base == NULL || transformer == NULL) return;\n\t{} tmp = *({}*)base;\n\ttmp ".format(
        func_name, transform_tuple[0], scalar_type, scalar_type) + transform_tuple[
        1] + "= *({}*)transformer;\n\t{}* sval = ({}*)malloc(sizeof({}));\n\t*sval = tmp;\n\t*storage = sval;\n}}".format(
        scalar_type, scalar_type, scalar_type, scalar_type)


def get_general_func_definition(transform_operations, scalar_type):
    func_name = produce_general_function_name(scalar_type)
    transform_operations = get_transform_operations()
    count_val = len(transform_operations)
    fptr_assignments = ""
    funcs_to_reference = []
    for t in transform_operations:
        funcs_to_reference.append(produce_function_name(t, scalar_type).split("(")[0])

    for i in range(0, count_val):
        fptr_assignments = fptr_assignments + "fptrs[{}] = &{};\n\t".format(str(i), funcs_to_reference[i])

    return "TransformerFunc* {} {{\n\t*count = {};\n\tTransformerFunc* fptrs = malloc(sizeof(TransformerFunc)*(*count));\n\t{}\n\treturn fptrs;\n}}\n\n".format(
        func_name, str(count_val), fptr_assignments)


def produce_transformers():
    fwd = os.path.dirname(os.path.realpath(__file__))
    with open(os.path.join(fwd, "transform.h"), "w+") as f:
        f.write(get_description_comment())
        f.write(get_includes())
        scalar_types = get_all_scalar_types()
        transform_operations = get_transform_operations()

        # Generate base transform functions
        for st in scalar_types:
            for t in transform_operations:
                f.write("void {};".format(produce_function_name(t, st)))
                f.write("\n")

        # Generate functions that return ptrs to transformer funcs
        for st in scalar_types:
            f.write("TransformerFunc* {};\n".format(produce_general_function_name(st)))

    with open(os.path.join(fwd, "transform.c"), "w+") as f:
        f.write(get_description_comment())
        f.write("#include \"transform.h\"\n\n")
        scalar_types = get_all_scalar_types()
        transform_operations = get_transform_operations()

        # Generate base transform functions
        for st in scalar_types:
            for t in transform_operations:
                f.write(get_func_definition(t, st))
                f.write("\n")

        # Generate functions that return ptrs to transformer funcs
        for st in scalar_types:
            f.write(get_general_func_definition(transform_operations, st))
            '''print(produce_general_function_name(st))
            for t in transform_operations:
                print(produce_function_name(t, st).split('(')[0])
            print("---")'''


if __name__ == "__main__":
    produce_transformers()
