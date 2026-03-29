; Minimal CRT for test-gen SDCC tests.
; Calls _main (returns in HL per z88dk), copies to DE, halts.
; z88dk-ticks: -end 3 stops at EX DE,HL. Grep '  hl=' for the result
; (HL has main's return value at that point).
; T-states: reported by ticks at -end 3 (excludes halt).
	SECTION CODE
	EXTERN _main
	call _main	; 3 bytes (addr 0-2)
	ex de,hl	; addr 3 — -end stops here, HL has result
	halt		; addr 4 — never reached with -end 3
