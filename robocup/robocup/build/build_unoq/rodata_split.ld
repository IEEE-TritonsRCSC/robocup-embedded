/* Auto-generated linker script fragment for LLEXT
 * Separates .rodata sections based on relocation status
 */

SECTIONS
{
    /* Read-only data WITH relocations - will be copied to RAM by LLEXT */
    .rodata : {
        *(.rodata._ZL12arduino_pins)
        *(.rodata._ZTV13BridgeMonitorILj512EE)
        *(.rodata._ZTVN7arduino12ZephyrSerialE)
    }

    /* Read-only data WITHOUT relocations - kept in flash by LLEXT */
    .llext.rodata.noreloc : {
        *(.rodata._ZN10RpcDecoderILj1024EE12get_responseIN7arduino7msgpack6object5nil_tEEEbjRT_R8RpcError.str1.1)
        *(.rodata._ZN11BridgeClass11update_safeEv.str1.1)
        *(.rodata._ZN13BridgeMonitorILj512EE5_readEj.str1.1)
        *(.rodata._ZN13BridgeMonitorILj512EE5writeEPKhj.str1.1)
        *(.rodata._ZN7arduino7msgpack6object3extC2EaPKhj.str1.1)
        *(.rodata._ZN7arduino7msgpack8Unpacker11deserializeINS_6StringEJEEEbRT_DpOT0_.str1.1)
        *(.rodata._ZN8RpcErrorC2Ev.str1.1)
        *(.rodata._ZNSt6vectorIjSaIjEE17_M_realloc_insertIJRjEEEvN9__gnu_cxx17__normal_iteratorIPjS1_EEDpOT_.str1.1)
        *(.rodata.str1.1)
    }

    /* Merge all .rel.rodata.* sections into .rel.rodata */
    .rel.rodata : {
        *(.rel.rodata)
        *(.rel.rodata.*)
    }

    /* Merge all .rela.rodata.* sections into .rela.rodata */
    .rela.rodata : {
        *(.rela.rodata)
        *(.rela.rodata.*)
    }

}
