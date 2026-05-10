/* Auto-generated linker script fragment for LLEXT
 * Separates .rodata sections based on relocation status
 */

SECTIONS
{
    /* Read-only data WITH relocations - will be copied to RAM by LLEXT */
    .rodata : {
        *(.rodata._ZTV13BridgeMonitorILj512EE)
        *(.rodata._ZTV15SerialTransport)
        *(.rodata._ZTV9BridgeUDPILj4096EE)
        *(.rodata._ZTVN7arduino12ZephyrSerialE)
        *(.rodata._ZTVN7arduino9IPAddressE)
    }

    /* Read-only data WITHOUT relocations - kept in flash by LLEXT */
    .llext.rodata.noreloc : {
        *(.rodata._ZN10RpcDecoderILj1024EE12get_responseIN7arduino7msgpack6object5nil_tEEEbjRT_R8RpcError.str1.1)
        *(.rodata._ZN11BridgeClass11update_safeEv.str1.1)
        *(.rodata._ZN11BridgeClass5beginEm.str1.1)
        *(.rodata._ZN13BridgeMonitorILj512EE5_readEj.str1.1)
        *(.rodata._ZN13BridgeMonitorILj512EE5writeEPKhj.str1.1)
        *(.rodata._ZN7RpcCallIJEE6resultIN7arduino7msgpack6object5nil_tEEEbRT_.part.0.str1.1)
        *(.rodata._ZN7RpcCallIJEEC2ERKN7arduino6StringEP9RPCClientP7k_mutexS8_.str1.1)
        *(.rodata._ZN7arduino5Print7printlnEv.str1.1)
        *(.rodata._ZN7arduino7msgpack6object3extC2EaPKhj.str1.1)
        *(.rodata._ZN7arduino7msgpack8Unpacker11deserializeINS_6StringEJEEEbRT_DpOT0_.str1.1)
        *(.rodata._ZN8RpcErrorC2Ev.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE11beginPacketEPKct.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE11parsePacketEv.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE4stopEv.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE5_readEj.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE5beginEt.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE5writeEPKhj.str1.1)
        *(.rodata._ZN9BridgeUDPILj4096EE9endPacketEv.str1.1)
        *(.rodata._ZNK7arduino9IPAddress9toString4Ev.str1.1)
        *(.rodata._ZNK7arduino9IPAddress9toString6Ev.str1.1)
        *(.rodata._ZNSt6vectorIjSaIjEE17_M_realloc_insertIJRjEEEvN9__gnu_cxx17__normal_iteratorIPjS1_EEDpOT_.str1.1)
        *(.rodata.loop.str1.1)
        *(.rodata.setup.str1.1)
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
