/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2009 Erwan Velu - All Rights Reserved
 *
 *   Permission is hereby granted, free of charge, to any person
 *   obtaining a copy of this software and associated documentation
 *   files (the "Software"), to deal in the Software without
 *   restriction, including without limitation the rights to use,
 *   copy, modify, merge, publish, distribute, sublicense, and/or
 *   sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following
 *   conditions:
 *
 *   The above copyright notice and this permission notice shall
 *   be included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 *
 * -----------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "hdt-cli.h"
#include "hdt-common.h"

void main_show_cpu(int argc __unused, char **argv __unused,
		   struct s_hardware *hardware)
{
    char features[81];
    cpu_detect(hardware);
    detect_dmi(hardware);
    more_printf("CPU\n");
    more_printf(" Manufacturer : %s \n", hardware->cpu.vendor);
    more_printf(" Product      : %s \n", hardware->cpu.model);
    more_printf(" CPU Cores    : %d \n", hardware->cpu.num_cores);
    more_printf(" L2 Cache     : %dK\n", hardware->cpu.l2_cache_size);

    memset(features, 0, sizeof(features));
    snprintf(features, sizeof(features), " Features     : %d Mhz : ",
	     hardware->dmi.processor.current_speed);
    if (hardware->cpu.flags.lm)
	strcat(features, "x86_64 64bit ");
    else
	strcat(features, "x86 32bit ");
    if (hardware->cpu.flags.smp)
	strcat(features, "SMP ");
    if (hardware->cpu.flags.vmx || hardware->cpu.flags.svm)
	strcat(features, "HwVIRT ");

    more_printf("%s\n", features);
}

static void show_flag(char *buffer, bool flag, char *flag_name, bool flush)
{
    char output_buffer[81];
    if ((((strlen(buffer) + strlen(flag_name)) > 66) && flag) || flush) {
	snprintf(output_buffer, sizeof output_buffer, "Flags     : %s\n",
		 buffer);
	more_printf("%s", output_buffer);
	memset(buffer, 0, sizeof(buffer));
	if (flush)
	    return;
    }
    if (flag)
	strcat(buffer, flag_name);
}

static void show_cpu(int argc __unused, char **argv __unused,
		     struct s_hardware *hardware)
{
    char buffer[81];
    reset_more_printf();
    more_printf("CPU\n");
    more_printf("Vendor    : %s\n", hardware->cpu.vendor);
    more_printf("Model     : %s\n", hardware->cpu.model);
    more_printf("CPU Cores : %d\n", hardware->cpu.num_cores);
    more_printf("L1 Cache  : %dK + %dK (I + D) \n",
		hardware->cpu.l1_instruction_cache_size,
		hardware->cpu.l1_data_cache_size);
    more_printf("L2 Cache  : %dK\n", hardware->cpu.l2_cache_size);
    more_printf("Family ID : %d\n", hardware->cpu.family);
    more_printf("Model  ID : %d\n", hardware->cpu.model_id);
    more_printf("Stepping  : %d\n", hardware->cpu.stepping);
    if (hardware->is_dmi_valid) {
	more_printf("FSB       : %d MHz\n",
		    hardware->dmi.processor.external_clock);
	more_printf("Cur. Speed: %d MHz\n",
		    hardware->dmi.processor.current_speed);
	more_printf("Max Speed : %d MHz\n", hardware->dmi.processor.max_speed);
	more_printf("Upgrade   : %s\n", hardware->dmi.processor.upgrade);
	more_printf("Voltage   : %d.%02d\n",
		    hardware->dmi.processor.voltage_mv / 1000,
		    hardware->dmi.processor.voltage_mv -
		    ((hardware->dmi.processor.voltage_mv / 1000) * 1000));
    }
    if (hardware->cpu.flags.smp) {
	more_printf("SMP       : yes\n");
    } else {
	more_printf("SMP       : no\n");
    }
    if (hardware->cpu.flags.lm) {
	more_printf("x86_64    : yes\n");
    } else {
	more_printf("x86_64    : no\n");
    }

    if (hardware->cpu.flags.vmx || hardware->cpu.flags.svm) {
	more_printf("HwVirt    : yes\n");
    } else {
	more_printf("HwVirt    : no\n");
    }

    memset(buffer, 0, sizeof(buffer));
    show_flag(buffer, hardware->cpu.flags.fpu, "fpu ", false);
    show_flag(buffer, hardware->cpu.flags.vme, "vme ", false);
    show_flag(buffer, hardware->cpu.flags.de, "de ", false);
    show_flag(buffer, hardware->cpu.flags.pse, "pse ", false);
    show_flag(buffer, hardware->cpu.flags.tsc, "tsc ", false);
    show_flag(buffer, hardware->cpu.flags.msr, "msr ", false);
    show_flag(buffer, hardware->cpu.flags.pae, "pae ", false);
    show_flag(buffer, hardware->cpu.flags.mce, "mce ", false);
    show_flag(buffer, hardware->cpu.flags.cx8, "cx8 ", false);
    show_flag(buffer, hardware->cpu.flags.apic, "apic ", false);
    show_flag(buffer, hardware->cpu.flags.sep, "sep ", false);
    show_flag(buffer, hardware->cpu.flags.mtrr, "mtrr ", false);
    show_flag(buffer, hardware->cpu.flags.pge, "pge ", false);
    show_flag(buffer, hardware->cpu.flags.mca, "mca ", false);
    show_flag(buffer, hardware->cpu.flags.cmov, "cmov ", false);
    show_flag(buffer, hardware->cpu.flags.pat, "pat ", false);
    show_flag(buffer, hardware->cpu.flags.pse_36, "pse_36 ", false);
    show_flag(buffer, hardware->cpu.flags.psn, "psn ", false);
    show_flag(buffer, hardware->cpu.flags.clflsh, "clflsh ", false);
    show_flag(buffer, hardware->cpu.flags.dts, "dts ", false);
    show_flag(buffer, hardware->cpu.flags.acpi, "acpi ", false);
    show_flag(buffer, hardware->cpu.flags.mmx, "mmx ", false);
    show_flag(buffer, hardware->cpu.flags.sse, "sse ", false);
    show_flag(buffer, hardware->cpu.flags.sse2, "sse2 ", false);
    show_flag(buffer, hardware->cpu.flags.ss, "ss ", false);
    show_flag(buffer, hardware->cpu.flags.htt, "ht ", false);
    show_flag(buffer, hardware->cpu.flags.acc, "acc ", false);
    show_flag(buffer, hardware->cpu.flags.syscall, "syscall ", false);
    show_flag(buffer, hardware->cpu.flags.mp, "mp ", false);
    show_flag(buffer, hardware->cpu.flags.nx, "nx ", false);
    show_flag(buffer, hardware->cpu.flags.mmxext, "mmxext ", false);
    show_flag(buffer, hardware->cpu.flags.lm, "lm ", false);
    show_flag(buffer, hardware->cpu.flags.nowext, "3dnowext ", false);
    show_flag(buffer, hardware->cpu.flags.now, "3dnow! ", false);
    show_flag(buffer, hardware->cpu.flags.svm, "svm ", false);
    show_flag(buffer, hardware->cpu.flags.vmx, "vmx ", false);
    show_flag(buffer, hardware->cpu.flags.pbe, "pbe ", false);
    show_flag(buffer, hardware->cpu.flags.fxsr_opt, "fxsr_opt ", false);
    show_flag(buffer, hardware->cpu.flags.gbpages, "gbpages ", false);
    show_flag(buffer, hardware->cpu.flags.rdtscp, "rdtscp ", false);
    show_flag(buffer, hardware->cpu.flags.pni, "pni ", false);
    show_flag(buffer, hardware->cpu.flags.pclmulqd, "pclmulqd ", false);
    show_flag(buffer, hardware->cpu.flags.dtes64, "dtes64 ", false);
    show_flag(buffer, hardware->cpu.flags.smx, "smx ", false);
    show_flag(buffer, hardware->cpu.flags.est, "est ", false);
    show_flag(buffer, hardware->cpu.flags.tm2, "tm2 ", false);
    show_flag(buffer, hardware->cpu.flags.sse3, "sse3 ", false);
    show_flag(buffer, hardware->cpu.flags.fma, "fma ", false);
    show_flag(buffer, hardware->cpu.flags.cx16, "cx16 ", false);
    show_flag(buffer, hardware->cpu.flags.xtpr, "xtpr ", false);
    show_flag(buffer, hardware->cpu.flags.pdcm, "pdcm ", false);
    show_flag(buffer, hardware->cpu.flags.dca, "dca ", false);
    show_flag(buffer, hardware->cpu.flags.xmm4_1, "xmm4_1 ", false);
    show_flag(buffer, hardware->cpu.flags.xmm4_2, "xmm4_2 ", false);
    show_flag(buffer, hardware->cpu.flags.x2apic, "x2apic ", false);
    show_flag(buffer, hardware->cpu.flags.movbe, "movbe ", false);
    show_flag(buffer, hardware->cpu.flags.popcnt, "popcnt ", false);
    show_flag(buffer, hardware->cpu.flags.aes, "aes ", false);
    show_flag(buffer, hardware->cpu.flags.xsave, "xsave ", false);
    show_flag(buffer, hardware->cpu.flags.osxsave, "osxsave ", false);
    show_flag(buffer, hardware->cpu.flags.avx, "avx ", false);
    show_flag(buffer, hardware->cpu.flags.hypervisor, "hypervisor ", false);
    show_flag(buffer, hardware->cpu.flags.ace2, "ace2 ", false);
    show_flag(buffer, hardware->cpu.flags.ace2_en, "ace2_en ", false);
    show_flag(buffer, hardware->cpu.flags.phe, "phe ", false);
    show_flag(buffer, hardware->cpu.flags.phe_en, "phe_en ", false);
    show_flag(buffer, hardware->cpu.flags.pmm, "pmm ", false);
    show_flag(buffer, hardware->cpu.flags.pmm_en, "pmm_en ", false);
    show_flag(buffer, hardware->cpu.flags.extapic, "extapic ", false);
    show_flag(buffer, hardware->cpu.flags.cr8_legacy, "cr8_legacy ", false);
    show_flag(buffer, hardware->cpu.flags.abm, "abm ", false);
    show_flag(buffer, hardware->cpu.flags.sse4a, "sse4a ", false);
    show_flag(buffer, hardware->cpu.flags.misalignsse, "misalignsse ", false);
    show_flag(buffer, hardware->cpu.flags.nowprefetch, "3dnowprefetch ", false);
    show_flag(buffer, hardware->cpu.flags.osvw, "osvw ", false);
    show_flag(buffer, hardware->cpu.flags.ibs, "ibs ", false);
    show_flag(buffer, hardware->cpu.flags.sse5, "sse5 ", false);
    show_flag(buffer, hardware->cpu.flags.skinit, "skinit ", false);
    show_flag(buffer, hardware->cpu.flags.wdt, "wdt ", false);
    show_flag(buffer, hardware->cpu.flags.ida, "ida ", false);
    show_flag(buffer, hardware->cpu.flags.arat, "arat ", false);
    show_flag(buffer, hardware->cpu.flags.tpr_shadow, "tpr_shadow ", false);
    show_flag(buffer, hardware->cpu.flags.vnmi, "vnmi ", false);
    show_flag(buffer, hardware->cpu.flags.flexpriority, "flexpriority ", false);
    show_flag(buffer, hardware->cpu.flags.ept, "ept ", false);
    show_flag(buffer, hardware->cpu.flags.vpid, "vpid ", false);

    /* Let's flush the remaining flags */
    show_flag(buffer, false, "", true);
}

struct cli_module_descr cpu_show_modules = {
    .modules = NULL,
    .default_callback = show_cpu,
};

struct cli_mode_descr cpu_mode = {
    .mode = CPU_MODE,
    .name = CLI_CPU,
    .default_modules = NULL,
    .show_modules = &cpu_show_modules,
    .set_modules = NULL,
};
