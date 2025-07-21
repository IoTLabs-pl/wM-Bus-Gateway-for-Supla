import logging
from pathlib import Path
from re import Match, compile

from esphome import codegen as cg
from esphome import config_validation as cv
from esphome.core import CORE
from esphome.components.esp32 import add_idf_component

from esphome.const import CONF_VERSION, CONF_PROJECT, CONF_NAME
from esphome.core import TimePeriod
from esphome.git import clone_or_update

from string import punctuation

CODEOWNERS = ["@kubasaw"]

_LOGGER = logging.getLogger(__name__)


COMPONENT_NAME = "supla-device"
RELATIVE_PATH_RE = compile(r"(?:\.\./)+[\w\-/.]+")
REPO_URL = "https://github.com/SUPLA/supla-device.git"
COMPONENTS_PATH = Path("extras/esp-idf")

CONFIG_SCHEMA = cv.All(
    cv.Schema({cv.GenerateID(CONF_VERSION): cv.string_strict}),
    cv.only_with_esp_idf,
)


def to_code(config):
    # This is a workaround for the fact that the supla-device component
    # is not a real idf component as it requires files outside of the
    # component directory...

    _REGISTERED_FILES: dict[str, Path] = {}

    cg.add_build_flag("-DSUPLA_DEVICE")
    cg.add_build_flag("-DSUPLA_DEVICE_ESP32")

    def normalize_path(base: Path, relative_path: Match[str]) -> str:
        # This is a workaround because platformio crashes when
        # inside components there are source files with the same name
        current_path = (base / relative_path[0]).resolve()

        if current_path.is_file() and current_path.suffix in {".cpp", ".c"}:
            if _REGISTERED_FILES.get(current_path.stem, current_path) != current_path:
                new_filename = f"{current_path.parent.stem}_{current_path.stem}"
                new_path = current_path.rename(current_path.with_stem(new_filename))
                _LOGGER.warning(f"Renamed {current_path} to {new_path}")
                current_path = new_path

            _REGISTERED_FILES[current_path.stem] = current_path

        return str(current_path)

    git_path, _ = clone_or_update(
        refresh=TimePeriod(days=7),
        url=REPO_URL,
        ref=config[CONF_VERSION],
        domain="supla",
    )
    git_path = Path(git_path)

    COMPONENT_PATH = git_path / COMPONENTS_PATH / COMPONENT_NAME
    CMAKELISTS_PATH = COMPONENT_PATH / "CMakeLists.txt"

    cmakelists_content = CMAKELISTS_PATH.read_text()

    cmakelists_content = RELATIVE_PATH_RE.sub(
        lambda path: normalize_path(COMPONENT_PATH, path), cmakelists_content
    )

    for cert in COMPONENT_PATH.glob("*_cert.pem"):
        cmakelists_content = cmakelists_content.replace(cert.name, "")

    CMAKELISTS_PATH.write_text(cmakelists_content)

    add_idf_component(
        name=COMPONENT_NAME,
        path=str(Path(git_path) / COMPONENTS_PATH / COMPONENT_NAME),
    )

    if project_conf := CORE.config.get(CONF_PROJECT):
        author, project = project_conf[CONF_NAME].split(".", 1)
        version = (
            project_conf[CONF_VERSION]
            .removeprefix("v")
            .replace("supla", "")
            .strip(punctuation)
        )

        cg.add_define("SUPLA_DEVICE_NAME", f"{author} {project}")
        cg.add_define("SUPLA_DEVICE_SW_VERSION", version)
        cg.add_define("SUPLA_DEVICE_HOSTNAME_PREFIX", project)
