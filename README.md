# Fife

Portable GUI framework and utilities.

## Project Layout

```
src/demo: A wee live demo of the framework. Contains main().
src/opt: Units that must be explicitly enabled, in local/config.mk.
src/lib: Unconditional units included in the library.
```

This project will build `libfife` and also `fifet`, our text editor.
I'm thinking the vast majority of the text editor's functionality will be implemented by `libfife`, so the actual app layer is thin enough to just ride along.

Once `fifet` works, I'll begin a new project `piccolo`, an image editor.

## TODO

- [ ] Generate public header.
- [ ] Test framework.
- [ ] Window Manager abstraction.
- - [ ] Copy/paste. Find an interface amenable to all particpants.
- [ ] Generic widget.
- - [ ] Events.
- - [ ] Focus ring.
- [ ] Window Manager implementations.
- - [x] X11
- - [ ] Wayland
- - [ ] MacOS
- - [ ] MS Windows
- [ ] Text rendering.
- - [ ] Tofu.
- [ ] Widgets.
- - [ ] Packer.
- - [ ] Label.
- - [ ] Button.
- - [ ] Text field.
- - [ ] Checkbox.
- - [x] Scroller. XXX All widgets have this capability.
- - [ ] Text editor.
- - [ ] Dialogue.
- - [ ] File dialogue.
- - [ ] Menu bar.
- [ ] Basic utilities.
- - [x] Filesystem. As an `opt` unit, in case we port to something exotic.
- - [x] Serial.
- - [ ] Image.
- [ ] Plugin framework. WAMR?
- [ ] 15-pixel font. (15 significant pixels; probly more like 20 with descenders)
