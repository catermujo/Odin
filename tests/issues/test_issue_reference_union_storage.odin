package main

Config :: struct {
    data: rawptr,
    color: [4]f32,
    size: [4]f32,
}
Value :: union {Config, string}
Command :: struct {data: Value}

replace :: proc(config: Config, data: ^int) -> Config {
    result := config
    result.data = data
    return result
}

main :: proc() {
    original, replacement: int = 456, 123
    commands := [2]Command{{Config{data = &original}}, {"before"}}
    // DUMBAI: A reference range and union case must both write through to the same original element.
    for &command in commands[:] {
        switch &config in command.data {
        case Config:
            config = replace(config, &replacement)
        case string:
            config = "after"
        }
    }
    assert((cast(^int)commands[0].data.(Config).data)^ == 123)
    assert(commands[1].data.(string) == "after")
    // DUMBAI: Value cases must still retain their copy when the source union changes during the case body.
    value: Value = Config{data = &original}
    switch config in value {
    case Config:
        value = "changed"
        assert((cast(^int)config.data)^ == 456)
    case string:
        assert(false)
    }
}
