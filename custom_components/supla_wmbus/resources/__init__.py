from esphome.helpers import write_file_if_changed
from requests import post
from pathlib import Path


def generate_resources():
    variables = {
        "frontend_script": "<script>"
        + post(
            "https://www.toptal.com/developers/javascript-minifier/api/raw",
            data={"input": Path(__file__).parent.joinpath("frontend.js").read_text()},
        ).text
        + "</script>",
    }

    h_content = "#pragma once\n" + "".join(
        f"extern const char* {var};\n" for var in variables
    )

    c_content = "".join(
        f'const char* {var} = "{content.replace('"', "'")}";\n'
        for var, content in variables.items()
    )

    dest_dir = Path(__file__).parent.parent

    write_file_if_changed(dest_dir / "resources.h", h_content)
    write_file_if_changed(dest_dir / "resources.cpp", c_content)


if __name__ == "__main__":
    generate_resources()
