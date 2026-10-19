.syntax unified
.thumb
.section .app_report,"ax",%progbits
.global th_app_report
.thumb_func
th_app_report:
 b.w th_report_main
.section .note.GNU-stack,"",%progbits
