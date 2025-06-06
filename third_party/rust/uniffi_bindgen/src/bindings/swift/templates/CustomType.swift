{%- let ffi_type_name=builtin|ffi_type|ref|ffi_type_name %}
{%- match config.custom_types.get(name.as_str())  %}
{%- when None %}
{#- No config, just forward all methods to our builtin type #}
/**
 * Typealias from the type name used in the UDL file to the builtin type.  This
 * is needed because the UDL type name is used in function/method signatures.
 */
public typealias {{ type_name }} = {{ builtin|type_name }}

#if swift(>=5.8)
@_documentation(visibility: private)
#endif
public struct FfiConverterType{{ name }}: FfiConverter {
    public static func read(from buf: inout (data: Data, offset: Data.Index)) throws -> {{ type_name }} {
        return try {{ builtin|read_fn }}(from: &buf)
    }

    public static func write(_ value: {{ type_name }}, into buf: inout [UInt8]) {
        return {{ builtin|write_fn }}(value, into: &buf)
    }

    public static func lift(_ value: {{ ffi_type_name }}) throws -> {{ type_name }} {
        return try {{ builtin|lift_fn }}(value)
    }

    public static func lower(_ value: {{ type_name }}) -> {{ ffi_type_name }} {
        return {{ builtin|lower_fn }}(value)
    }
}

{%- when Some(config) %}

{# When the config specifies a different type name, create a typealias for it #}
{%- if let Some(concrete_type_name) = config.type_name %}
/**
 * Typealias from the type name used in the UDL file to the custom type.  This
 * is needed because the UDL type name is used in function/method signatures.
 */
public typealias {{ type_name }} = {{ concrete_type_name }}
{%- endif %}

{%- if let Some(imports) = config.imports %}
{%- for import_name in imports %}
{{ self.add_import(import_name) }}
{%- endfor %}
{%- endif %}

#if swift(>=5.8)
@_documentation(visibility: private)
#endif
public struct FfiConverterType{{ name }}: FfiConverter {
    {#- Custom type config supplied, use it to convert the builtin type #}

    public static func read(from buf: inout (data: Data, offset: Data.Index)) throws -> {{ type_name }} {
        let builtinValue = try {{ builtin|read_fn }}(from: &buf)
        return {{ config.lift("builtinValue") }}
    }

    public static func write(_ value: {{ type_name }}, into buf: inout [UInt8]) {
        let builtinValue = {{ config.lower("value") }}
        return {{ builtin|write_fn }}(builtinValue, into: &buf)
    }

    public static func lift(_ value: {{ ffi_type_name }}) throws -> {{ type_name }} {
        let builtinValue = try {{ builtin|lift_fn }}(value)
        return {{ config.lift("builtinValue") }}
    }

    public static func lower(_ value: {{ type_name }}) -> {{ ffi_type_name }} {
        let builtinValue = {{ config.lower("value") }}
        return {{ builtin|lower_fn }}(builtinValue)
    }
}

{%- endmatch %}

{#
We always write these public functions just incase the type is used as
an external type by another crate.
#}
#if swift(>=5.8)
@_documentation(visibility: private)
#endif
public func FfiConverterType{{ name }}_lift(_ value: {{ ffi_type_name }}) throws -> {{ type_name }} {
    return try FfiConverterType{{ name }}.lift(value)
}

#if swift(>=5.8)
@_documentation(visibility: private)
#endif
public func FfiConverterType{{ name }}_lower(_ value: {{ type_name }}) -> {{ ffi_type_name }} {
    return FfiConverterType{{ name }}.lower(value)
}

