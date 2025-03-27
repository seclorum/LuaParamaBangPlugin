# Note: edit JUCE Doxyfile to GENERATE_XML = YES

import xml.etree.ElementTree as ET
import sys
import os

# Get the Doxygen XML path from the argument
doxygen_xml_path = sys.argv[1]

# Ensure the XML path exists
if not os.path.isdir(doxygen_xml_path):
    print(f"Error: Invalid Doxygen XML directory: {doxygen_xml_path}")
    sys.exit(1)

# Load the Doxygen XML
tree = ET.parse(os.path.join(doxygen_xml_path, "index.xml"))
root = tree.getroot()

# Iterate over the classes in the XML and generate LuaBridge bindings
def generate_luabridge_bindings():
    lua_bridge_code = []

    for compound in root.findall(".//compound"):
        compound_name = compound.find("compoundname").text
        if compound.attrib["kind"] == "class":
            lua_bridge_code.append(f"// Binding for {compound_name}")
            lua_bridge_code.append(f"LuaBridge::getGlobalNamespace(L)")

            # Begin class binding
            lua_bridge_code.append(f"    .beginClass<{compound_name}>(\"{compound_name}\")")

            # Iterate through the functions of the class
            for section in compound.findall(".//sectiondef[@kind='public']"):
                for member in section.findall("memberdef"):
                    if member.attrib["kind"] == "function":
                        function_name = member.find("name").text
                        lua_bridge_code.append(f"        .addFunction(\"{function_name}\", &{compound_name}::{function_name})")

            # End class binding
            lua_bridge_code.append("        .endClass();\n")

    # Output the generated LuaBridge code
    output_file = "generated_luabridge_bindings.cpp"
    with open(output_file, "w") as f:
        f.write("\n".join(lua_bridge_code))
    print(f"LuaBridge bindings generated and saved to {output_file}")

# Generate the bindings
generate_luabridge_bindings()

