"""Compile public examples/headers and run the sealed clock with actual dependencies.

Only Arduino's String declaration is stubbed for the native Units build. Timing,
System, Units and Observable all come from coordinated source checkouts.
"""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--dependencies", type=Path, required=True)
parser.add_argument("--compiler", default="g++")
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
includes = [root / "src", root / "tests/stubs"]
includes += [args.dependencies.resolve() / f"ESPressio-{name}/src"
             for name in ("System", "Units", "Observable")]
flags = [args.compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror"]
flags += [f"-I{path}" for path in includes]

with tempfile.TemporaryDirectory() as directory:
    temporary = Path(directory)

    def compile_text(name, text, diagnostic=None):
        source = temporary / (name + ".cpp")
        source.write_text(text)
        result = subprocess.run(flags + ["-c", str(source), "-o", str(temporary / (name + ".o"))],
                                capture_output=True, text=True)
        if diagnostic is None:
            if result.returncode:
                raise RuntimeError(result.stderr)
        elif result.returncode == 0 or diagnostic not in result.stderr:
            raise RuntimeError("Expected diagnostic absent: " + str(diagnostic) + "\n" + result.stderr)
        print(name + " passed", flush=True)

    for index, code in enumerate(re.findall(r"```cpp\n(.*?)```", (root / "README.md").read_text(), re.S)):
        compile_text(f"readme_{index}", code + "\nint main() {}\n")
    for header in ("TimeReliability", "ClockUncertainty", "ClockModelSnapshot", "ClockSynchronization",
                   "ClockRegression", "ClockDiscipline", "IClockSynchronizationTarget", "ISystemClock",
                   "ISystemClockObserver", "SystemClock", "TimingSystemClock"):
        compile_text(header, f"#include <ESPressio_{header}.hpp>\nint main() {{}}\n")
    compile_text("invalid_capacity", "#include <ESPressio_ClockDiscipline.hpp>\n"
                 "ESPressio::Timing::ClockDiscipline<3> invalid;\n", "at least four")
    compile_text("unrestricted_rebase_removed", "#include <ESPressio_TimingSystemClock.hpp>\n"
                 "int main() { ESPressio::Timing::SystemClock<>::GetInstance().SetTime({}); }\n", "SetTime")
    binary = temporary / "real_clock"
    subprocess.run(flags + ["-DESPRESSIO_TIMING_REAL_DEPENDENCIES=1",
                   str(root / "tests/test_synchronized_system_clock.cpp"), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    print("actual dependency sealed clock / allocation denial passed", flush=True)
