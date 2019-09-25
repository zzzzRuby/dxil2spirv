if __name__ == "__main__":
    import subprocess
    import os
    import sys
    import shutil
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('--dxc', dest='dxc')
    parser.add_argument('--spirv-dis', dest='spirv_dis')
    parser.add_argument('--generate-spirv', action='store_true', default=False, dest='generate_spirv')
    parser.add_argument('--generate-dxil', action='store_true', default=False, dest='generate_dxil')
    parser.add_argument('--generate-header', action='store_true', default=False, dest='generate_header')
    parser.add_argument('--output', dest='output')
    parser.add_argument('--input', dest='input')
        
    options = parser.parse_args(sys.argv[1:])

    rootdir = os.path.abspath(".")
    if os.path.exists(options.output):
        shutil.rmtree(options.output)

    os.makedirs(options.output)

    shaderlist = [
        "#include <array>",
        "#include <string_view>",
        "typedef struct dxil2spirv_test {",
        "\tstd::string name;",
        "\tstd::string hlsl;",
        "\tstd::wstring entry;",
        "\tstd::wstring profile;",
        "} dxil2spirv_test;",
        "using namespace std::string_literals;"
    ]
    shadernames = []
    def dxcompile(name, profile, entry):
        global shaderlist
        global shadernames
        profile_version = "6_3"
        if options.generate_spirv:
            subprocess.run([
                options.dxc,
                os.path.join(options.input, f"{name}.hlsl"),
                "-Fo",
                os.path.join(options.output, f"{name}.spv"),
                "-T",
                f"{profile}_{profile_version}",
                "-E",
                entry,
                "-spirv"
            ])
            if options.spirv_dis:
                subprocess.run([
                    options.spirv_dis,
                    os.path.join(options.output, f"{name}.spv"),
                    "-o",
                    os.path.join(options.output, f"{name}.spv.asm")
                ])
        if options.generate_dxil:
            subprocess.run([
                options.dxc,
                os.path.join(options.input, f"{name}.hlsl"),
                "-Fo",
                os.path.join(options.output, f"{name}.dxil"),
                "-T",
                f"{profile}_{profile_version}",
                "-Fc",
                "-E",
                entry,
                os.path.join(options.output, f"{name}.dxil.asm")
            ])
        
        if options.generate_header:
            with open(os.path.join(options.input, f"{name}.hlsl"), "r") as f:
                hlsl = f.read()

                with open(os.path.join(options.output, f"{name}.hlsl.h"), "w") as f2:
                    f2.write(f'static const std::string {name}_hlsl = u8R"({hlsl})";\n')
                    f2.write(f'static const std::wstring {name}_hlsl_entry = LR"({entry})";\n')
                    f2.write(f'static const std::wstring {name}_hlsl_profile = LR"({profile}_{profile_version})";')
            shaderlist.append(f'#include "{name}.hlsl.h"')
            shadernames.append(name)

    dxcompile("types_test", "vs", "main")

    if options.generate_header:
        with open(os.path.join(options.output, "dxil2spirv_test.h"), "w") as f2:
            shaderlist.append(f'static const std::array<dxil2spirv_test, {len(shadernames)}> dxil2spirv_test_instances = {{')
            for name in shadernames:
                shaderlist.append(f'\t{{ u8R"({name})", {name}_hlsl, {name}_hlsl_entry, {name}_hlsl_profile }},')
            shaderlist.append('};')
            f2.write('\n'.join(shaderlist))
    
