import logging
from pathlib import Path
from re import compile
from string import punctuation

from esphome import codegen as cg
from esphome import config_validation as cv
from esphome.components.esp32 import add_idf_component
from esphome.const import CONF_ESPHOME, CONF_ID, CONF_NAME, CONF_PROJECT, CONF_VERSION
from esphome.core import CORE, TimePeriod
from esphome.git import clone_or_update

CODEOWNERS = ["@kubasaw"]

_LOGGER = logging.getLogger(__name__)

SuplaDeviceComponent = cg.esphome_ns.namespace("supla_device").class_(
    "SuplaDeviceComponent", cg.Component
)

COMPONENT_NAME = "supla-device"
RELATIVE_PATH_RE = compile(r"(?P<path>(?:\.\./)+[\w\-/.]+)")
CMAKE_SRC_RE = compile(r"\$\{SUPLA_(?:DEVICE|COMMON)_SRC_DIR\}/(?P<path>[\w\-/.]+)")
REPO_URL = "https://github.com/SUPLA/supla-device.git"
COMPONENTS_PATH = Path("extras/esp-idf")

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(SuplaDeviceComponent),
            cv.GenerateID(CONF_VERSION): cv.string_strict,
        }
    ),
    cv.only_with_esp_idf,
)


async def to_code(config):
    # This is a workaround for the fact that the supla-device component
    # is not a real idf component as it requires files outside of the
    # component directory...
    # The standard way to add an idf component is to use
    # `esp32.components` config option in yaml

    registered_files: dict[str, Path] = {}

    cg.add_build_flag("-DSUPLA_DEVICE")
    cg.add_build_flag("-DSUPLA_DEVICE_ESP32")

    def deduplicate(path: Path) -> Path:
        # This is a workaround because platformio crashes when
        # inside components there are source files with the same name
        if path.is_file() and path.suffix in {".cpp", ".c"}:
            if registered_files.get(path.stem, path) != path:
                new_name = f"{path.parent.stem}_{path.stem}"
                path = path.rename(path.with_stem(new_name))
                _LOGGER.warning(f"Renamed to {path}")
            registered_files[path.stem] = path
        return path

    git_path, _ = clone_or_update(
        refresh=TimePeriod(days=7),
        url=REPO_URL,
        ref=config[CONF_VERSION],
        domain="supla",
    )
    git_path = Path(git_path)

    component_path = git_path / COMPONENTS_PATH / COMPONENT_NAME
    cmake_dir     = git_path / "cmake"
    src_dir       = git_path / "src"
    cmakelists    = component_path / "CMakeLists.txt"

    # Strip ${COMPONENT_DIR}/ and embedded cert filenames before path resolution
    content = cmakelists.read_text().replace("${COMPONENT_DIR}/", "")
    for cert in component_path.glob("*_cert.pem"):
        content = content.replace(cert.name, "")
    cmakelists.write_text(content)

    for cmake_file, base_dir, regex in [
        (cmakelists,                             component_path,           RELATIVE_PATH_RE),
        (cmake_dir / "SuplaDeviceSources.cmake", src_dir,                  CMAKE_SRC_RE),
        (cmake_dir / "SuplaCommonSources.cmake", src_dir / "supla-common", CMAKE_SRC_RE),
    ]:
        content = cmake_file.read_text()
        content = regex.sub(
            lambda m, d=base_dir: str(deduplicate((d / m["path"]).resolve())), content
        )
        cmake_file.write_text(content)

    add_idf_component(
        name=COMPONENT_NAME,
        path=str(component_path),
    )

    if project_conf := CORE.config[CONF_ESPHOME].get(CONF_PROJECT):
        author, project = project_conf[CONF_NAME].split(".", 1)
        version = (
            project_conf[CONF_VERSION]
            .replace("supla", "")
            .strip(punctuation)
            .removeprefix("v")
        )
        cg.add_define("SUPLA_DEVICE_NAME", f"{author} {project}")
        cg.add_define("SUPLA_DEVICE_SW_VERSION", version)
        cg.add_define("SUPLA_DEVICE_HOSTNAME_PREFIX", project)
        cg.add_define("SUPLA_DEVICE_LIBRARY_VERSION", config[CONF_VERSION])

    supla_device = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(supla_device, config)
