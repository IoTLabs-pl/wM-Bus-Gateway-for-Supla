function add_meter(dialog) {
    const [id, driver] = dialog.querySelectorAll('input,select');
    if (!id.reportValidity())
        return;

    const meter = document.getElementById("meter_template").content.firstElementChild.cloneNode(true);
    const key = meter.querySelector('input[type="text"]');
    meter.querySelector('h3').firstChild.nodeValue = id.value + " " + driver.value;
    meter.querySelector('input[type="checkbox"]').value = id.value + "," + driver.value;
    key.name = "key_" + id.value;

    document.getElementById("new_meter").before(meter);
    dialog.close();
    key.focus();
}

window.addEventListener("DOMContentLoaded", () => {
    const dialog = document.getElementById("meter_dialog");
    document.body.append(dialog);
    document.getElementById("new_meter").addEventListener("click", () => dialog.showModal());
    dialog.querySelector('button').addEventListener("click", () => add_meter(dialog));
});
