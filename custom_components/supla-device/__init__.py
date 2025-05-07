from esphome import config_validation as cv, codegen as cg
from esphome.core import TimePeriod
from esphome.components.esp32 import add_idf_component
from esphome.const import CONF_ID
from esphome.git import clone_or_update
from pathlib import Path
from re import compile, Match
from os import rename
import logging

from .resources import generate_resources

_LOGGER = logging.getLogger(__name__)


COMPONENT_NAME = "supla-device"


SuplaDevice = cg.global_ns.class_("SuplaDevice")
SuplaComponent = cg.esphome_ns.namespace("supla_wmbus_reader").class_(
    "Component", cg.Component
)
wmbus_radio = cg.esphome_ns.namespace("wmbus_radio").class_("Radio", cg.Component)

wmbus_reader_ns = cg.esphome_ns.namespace("wmbus_reader")
ScreenManager = wmbus_reader_ns.class_("DisplayScreenManager")

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SuplaComponent),
        cv.GenerateID("radio_id"): cv.use_id(wmbus_radio),
        cv.GenerateID("manager_id"): cv.use_id(ScreenManager),
    }
)


def prepare_component():
    # This is a workaround for the fact that the supla-device component
    # is not a real idf component as it requires files outside of the
    # component directory...
    RELATIVE_PATH_RE = compile(r"(?:\.\./)+[\w\-/.]+")

    URL = "https://github.com/SUPLA/supla-device.git"
    REF = "v25.2"
    COMPONENTS_PATH = Path("extras/esp-idf")

    _REGISTERED_FILES = {}

    cg.add_build_flag("-DSUPLA_DEVICE")
    cg.add_build_flag("-DSUPLA_DEVICE_ESP32")

    def normalize_path(base: Path, relative_path: Match[str]) -> str:
        # This is a workaround because platformio crashes when
        # inside components there are source files with the same name
        relative_path = relative_path[0]
        canonical_path = base.joinpath(relative_path).resolve()

        if canonical_path.is_file() and canonical_path.suffix in {".cpp", ".c"}:
            if (
                _REGISTERED_FILES.get(canonical_path.stem, canonical_path)
                != canonical_path
            ):
                fq_stem = f"{canonical_path.parent.stem}_{canonical_path.stem}"
                canonical_path = canonical_path.with_stem(fq_stem)
                rename(base.joinpath(relative_path), canonical_path)
                _LOGGER.warning(f"Renamed {relative_path} to {canonical_path}")

            _REGISTERED_FILES[canonical_path.stem] = canonical_path

        return str(canonical_path)

    git_path, _ = clone_or_update(
        refresh=TimePeriod(days=7),
        url=URL,
        ref=REF,
        domain="idf_components",
    )

    COMPONENT_PATH = git_path.joinpath(COMPONENTS_PATH, COMPONENT_NAME)
    CMAKELISTS_PATH = COMPONENT_PATH.joinpath("CMakeLists.txt")

    cmakelists_content = CMAKELISTS_PATH.read_text()

    cmakelists_content = RELATIVE_PATH_RE.sub(
        lambda path: normalize_path(COMPONENT_PATH, path), cmakelists_content
    )

    for cert in COMPONENT_PATH.glob("*_cert.pem"):
        cmakelists_content = cmakelists_content.replace(cert.name, "")

    CMAKELISTS_PATH.write_text(cmakelists_content)

    add_idf_component(
        name=COMPONENT_NAME,
        repo=URL,
        ref=REF,
        path=COMPONENTS_PATH,
        components=[COMPONENT_NAME],
        refresh=TimePeriod(days=7),
    )


async def to_code(config):
    cg.add_define("USE_WMBUS_METER_SENSOR")

    prepare_component()

    # generate_resources()

    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_radio(await cg.get_variable(config["radio_id"])))
    cg.add(var.set_display_manager(await cg.get_variable(config["manager_id"])))
    await cg.register_component(var, config)
