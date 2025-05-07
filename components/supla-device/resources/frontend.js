function attach_handlers() {
    document.querySelectorAll(".meter_field").forEach((f) => {
        f.oninput = update_meters;
    });
}
function add_meter() {
    const last_meter = document.querySelector(".meter.box:last-of-type");
    const new_meter = last_meter.cloneNode(true);
    new_meter.querySelectorAll(".meter_field").forEach((f) => {
        if (f.classList.contains("extra")) f.closest(".form-field").remove();
        else f.value = "";
    });
    new_meter.querySelector('h3').onclick = () => new_meter.classList.toggle('collapsed');
    last_meter.after(new_meter);
    update_meters();
}
function remove_meter() {
    event.target.closest(".meter.box").remove();
    update_meters();
}
function update_meters() {
    document.querySelectorAll(".meter.box").forEach((meter, index) => {
        meter.querySelector("h3").innerText = "Meter " + index;
        cfg = meter.querySelector("input.meter_cfg");
        cfg.name = "meter_" + index;
        cfg.value = Array.from(meter.querySelectorAll(".meter_field"))
            .map((f) => f.value)
            .join(",");
    });
    attach_handlers();
}
window.addEventListener("load", update_meters);