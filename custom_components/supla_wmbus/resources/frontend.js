function bind_labels_with_inputs(root = document, id_prefix = '', force = false, set_name = true) {
    const selector_suffix = force ? '' : ':not([for])'

    const labels = Array.from(root.querySelectorAll('.form-field>label' + selector_suffix));

    labels.forEach(label => {
        const input = label.nextElementSibling;
        const id = [id_prefix, ...label.textContent.split(' ')].filter((e) => !!e).join('_').toLowerCase();
        [
            [label, 'htmlFor'],
            [input, 'id'],
            set_name ? [input, 'name'] : [],
        ].forEach(([el, attr]) => el && (el[attr] = id));
    });
}

function enumerate_meter(meter, idx) {
    const cfg_input = meter.querySelector('input[type="hidden"]');
    const label = "meter_" + idx;
    cfg_input.name = label;

    bind_labels_with_inputs(meter, label, true, false);

    meter.querySelector("h3").innerText = "Meter " + idx;

    return meter;
}

function enumerate_meters() {
    return Array.from(document.querySelectorAll(".meter.box")).map(enumerate_meter);
}

function drop_bind_fields(meter) {
    meter.querySelectorAll(".form-field").forEach((f, i) => {
        if (i < 3 || f.querySelector('button'))
            f.querySelectorAll('input,select').forEach(el => el.value = '');
        else
            f.remove();
    });
    return meter;
}

function add_meter() {
    const meters = document.querySelectorAll(".meter.box");

    const last_meter = meters[meters.length - 1];
    const new_meter = [
        drop_bind_fields,
        attach_meter_box_handlers,
        attach_meter_box_collapser,
        mark_indexable_labels,
        enumerate_meter
    ].reduce((m, f) => f(m, meters.length), last_meter.cloneNode(true));

    last_meter.after(new_meter);

    return new_meter;
}

function mark_indexable_labels(meter_box) {
    const labels = meter_box.querySelectorAll('label[data-indexable]');
    labels.forEach(label => {
        if (!label.querySelector('em'))
            label.innerHTML += " <em>(%d-th)</em>";
    })

    return meter_box;
}

function attach_meter_box_handlers(meter_box) {
    const remove_button = meter_box.querySelector('button');
    remove_button.addEventListener('click', () => {
        meter_box.remove();
        enumerate_meters();
    });

    const cfg_input = meter_box.querySelector('input[type="hidden"]');
    const other_inputs = Array.from(meter_box.querySelectorAll('input,select')).filter(input => input !== cfg_input);

    const config_updater = () => {
        cfg_input.value = other_inputs.map(input => input.value).join(",");
    }
    other_inputs.forEach(input => {
        input.addEventListener('input', config_updater);
    });

    config_updater();

    return meter_box;
}

function attach_meter_box_collapser(meter_box) {
    const title = meter_box.querySelector("h3");
    title.addEventListener("click", () => {
        meter_box.classList.toggle('collapsed');
    });

    return meter_box;
}

function on_load() {
    document.getElementById("add_meter").addEventListener("click", add_meter);
    enumerate_meters().map(attach_meter_box_handlers).map(mark_indexable_labels);

    bind_labels_with_inputs();
}

window.addEventListener("DOMContentLoaded", on_load);