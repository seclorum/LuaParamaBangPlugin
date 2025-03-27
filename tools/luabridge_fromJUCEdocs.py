# Note: edit JUCE Doxyfile to GENERATE_XML = YES

import xml.etree.ElementTree as ET

# Path to the Doxygen XML output
doxygen_xml_path = "path/to/doxygen/xml/index.xml"

# Load the Doxygen XML
tree = ET.parse(doxygen_xml_path)
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
    with open("generated_luabridge_bindings.cpp", "w") as f:
        f.write("\n".join(lua_bridge_code))
    print("LuaBridge bindings generated!")

# Generate the bindings
generate_luabridge_bindings()

