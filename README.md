# Fife

Portable GUI framework and utilities.

## Project Layout

```
src/demo: A wee live demo of the framework. Contains main().
src/opt: Units that must be explicitly enabled, in local/config.mk.
src/lib: Unconditional units included in the library.
```

## TODO

- [ ] Generate public header.
- [ ] Test framework.
- [ ] Window Manager abstraction.
- - [ ] Copy/paste. Find an interface amenable to all particpants.
- [ ] Generic widget.
- [ ] Window Manager implementations.
- - [x] X11
- - [ ] Wayland
- - [ ] MacOS
- - [ ] MS Windows
- [ ] Widgets.
- - [ ] Packer.
- - [ ] Label.
- - [ ] Button.
- - [ ] Text field.
- - [ ] Checkbox.
- - [ ] Scroller.
- - [ ] Text editor.
- - [ ] Dialogue.
- - [ ] File dialogue.
- - [ ] Menu bar.
- [ ] Basic utilities.
- - [x] Filesystem. As an `opt` unit, in case we port to something exotic.
- - [x] Serial.
- - [ ] Image.
- [ ] Plugin framework. WAMR?
