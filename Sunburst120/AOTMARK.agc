### FILE="Main.annotation"
# Copyright:	Public domain.
# Filename:	AOTMARK.agc
# Purpose:	A module for revision 0 of BURST120 (Sunburst).
#		It is part of the source code for the Lunar Module's (LM)
#		Apollo Guidance Computer (AGC) for Apollo 5.
# Assembler:	yaYUL
# Contact:	Ron Burkey <info@sandroid.org>.
# Website:	www.ibiblio.org/apollo/index.html
# Mod history:	2016-09-30 RSB	Created draft version.
#		2016-10-05 RSB	Completed transcription.

# Page 159
		BANK	13
		EBANK=	AOTAZ
AOTMARK		INHINT
		TS	RUPTREG2	# BIT14=INFLIGHT  0=NONFLIGHT
		CCS	MARKSTAT	# ARE MARKS BUTTONS IN USE
		TC	+2		# MARKS BUTTONS NO AVAILABLE
		TC	MKVAC		# FIND A VAC AREA.
		TC	ALARM
		OCT	00105
		TC	ENDOFJOB
		
MKVAC		CCS	VAC1USE	
		TCF	MKVACFND
		CCS	VAC2USE
		TCF	MKVACFND
		CCS	VAC3USE
		TCF	MKVACFND
		CCS	VAC4USE
		TCF	MKVACFND
		CCS	VAC5USE
		TCF	MKVACFND
		TC	ABORT		# VAC AREAS OCCUPIED
		OCT	01207
		
MKVACFND	AD	TWO
		TS	MARKSTAT		
		CAF	ZERO
		TS	XYMARK		# INITIALIZE MARK ID REGISTER
		INDEX	MARKSTAT
		TS	0 -1		# SHOW VAC OCCUPIED

		CA	RUPTREG2
		ADS	MARKSTAT

		CAF	PRIO32
		TC	NOVAC
		EBANK=	AOTAZ
		2CADR	GETMKS
		
		TC	MODEEXIT

MKRELEAS	CAF	ZERO
		XCH	MARKSTAT	# SET MARKSTAT TO ZERO
		CCS	A
		INDEX	A
		TS	0		# SHOW VAC AREA AVAILABLE
		TC	SWRETURN
		
# Page 160
GETMKS		CAF	ZERO
		TS	XYMARK
		CAF	BIT14		# FIND IF IN-FLIGHT OR NON-FLIGHT
		MASK	MARKSTAT
		CCS	A
		TC	MKVB53		# IN-FLIGHT
		CS	BIT11		# NON-FLIGHT, SET Y MARK =1
		MASK	MARKSTAT
		AD	BIT11
		TS	MARKSTAT
		CAF	VB51		# REQUEST X MARK
		TC	+2
VKVB53		CAF	VB53		# REQUEST X AND Y MARKS
		TC	NVSBWAIT
		TC	FLASHON
		TC	ENDIDLE
		TC	GETMKS	+1
		TC	GETMKS	+1
		
MKCHEK		CAF	BIT10		# WAS X MARK MADE
		MASK	MARKSTAT
		CCS	A
		TC	+2		# X MARK MADE-LOOK AT Y MARK
		TC	MKALARM
		CCS	A
		TC	GETDET		# BOTH MARKS MADE-SHOW ACCEPT-GET DETENT
		
MKALARM		TC	ALARM		# IF EITHER MARK IS MISSING AFTER ENTER
		OCT	111
		CS	PRIO17		# SET BITS 10,11,12,13 TO ZERO IN MARKSTAT
		MASK	MARKSTAT
		TS	MARKSTAT
		TC	GETMKS		# GETMKS AGAIN
		
GETDET		CS	BIT12
		MASK	MARKSTAT
		AD	BIT12		# SET BIT12=1 TO SHOW MARKS ACCEPTED
		TS	MARKSTAT
DETVB21		CAF	V21N43E
		TC	NVSBWAIT
		TC	ENDIDLE
		TC	DETVB21
		TC	DETVB21
		
		CS	OCT3
		MASK	DETCODE
		CCS	A
		TC	DETVB21		# BAD CODE, REQUEST AGAIN
		
# Page 161
		TC	+3		# LOOKS GOOD BUT COULD BE ZERO
		TC	+1
		TC	DETVB21
		CAF	OCT3
		MASK	DETCODE		# SEE IF ZERO
		CCS	A
		TC	+2		# GOOD CODE-STORE ANGLES
		TC	DETVB21		# ZERO-BAD CODE-REQUEST AGAIN
		
		CAF	LOW9
		MASK	MARKSTAT
		TS	BASVAC		# STORE VAC AREA ADDRESS
		INDEX	DETCODE
		CA	AOTEL	-1
		INDEX	BASVAC
		TS	9D		# STORE ELV ANGLE IN VAC +9
		INDEX	DETCODE
		CA	AOTAZ	-1
		INDEX	BASVAC
		TS	8D		# STORE AZIMUTH IN VAC +8
		
		CA	AOTAZ	+1	# COMPENSATION FOR THE APPARENT TILT OF
		EXTEND			# AOT FIELD OF VIEW IN THE LEFT AND RIGHT
		INDEX	DETCODE		# DETENTS IS STORED IN LOC 10D OF VAC
		MSU	AOTAZ	-1	# IN ONES COMPLEMENT
		INDEX	BASVAC
		TS	10D		# SINGLE PREC. TILT COMPENSATION ANGLE
		
		CAF	BIT14		# IN-FLIGHT OR NON-FLIGHT
		MASK	MARKSTAT
		CCS	A
		TC	MKOUT		# IN-FLIGHT, NO MORE DATA NEEDED
		TC	ROTVB24		# NON-FLIGHT, GET ROTATION ANGLES
		
ROTVB24		CAF	V24N42E		# REQUEST RETICLE ROTATION ANGLES
		TC	NVSBWAIT
		TC	ENDIDLE
		TC	ROTVB24
		TC	ROTVB24
		
		CAF	LOW9
		MASK	MARKSTAT
		TS	BASVAC
		CA	DSPTEM2		# YROT
		INDEX	BASVAC
		TS	3		# STORE Y RETICLE ROT ANGLE IN VAC +3
		CA	DSPTEM2	+1	# SROT
		INDEX	BASVAC
		TS	5		# STORE SPIRAL ROT ANGLE VAC +5
		
# Page 162
MKOUT		CAF	LOW9
		MASK	MARKSTAT
		TS	MARKSTAT	# SET HI5 ZERO
		
		INHINT
		CAF	ONE
		TC	WAITLIST
		EBANK=	AOTAZ
		2CADR	ENDMARKS
		
		TC	ENDOFJOB
		
ENDMARKS	CAF	ONE
		TCF	GOODEND

# Page 163
MARKRUPT	TS	BANKRUPT
		CA	CDUY		# STORE CDUS AND TIME NOW -- THEN SEE IF
		TS	ITEMP3		# WE NEED THEM
		CA	CDUZ
		TS	ITEMP4
		CA	CDUX
		TS	ITEMP5
		EXTEND
		DCA	TIME2
		DXCH	ITEMP1
		XCH	Q
		TS	QRUPT
		
		CAF	OCT34		# SEE IF X OR Y MARK OR MKREJECT
		EXTEND
		RAND	NAVKEYIN
		CCS	A
		TCF	+2		# ITS A LIVE ONE-CHECK FOR ACCEPT
		TCF	SOMEKEY		# SOME OTHER KEY
		
		CAF	BIT12		# HAVE MARKS BEEN ACCEPTED
		MASK	MARKSTAT
		CCS	A
		TC	RESUME		# MARKS MADE AND ACCEPTED - DO NOTHING

		CCS	MARKSTAT	# ARE MARKS BEING ACCEPTED
		TC	FINDKEY
		TC	ALARM		# MARKS NOT BEING ACCEPTED
		OCT	112
		TC	RESUME
		
FINDKEY		CAF	BIT5
		EXTEND
		RAND	NAVKEYIN	# CHANNEL 16
		CCS	A
		TCF	MKREJ		# ITS A MARK REJECT
		CAF	BIT4
		EXTEND
		RAND	NAVKEYIN
		CCS	A
		TCF	YMKRUPT		# ITS A Y MARK
		CAF	BIT3
		EXTEND
		RAND	NAVKEYIN
		CCS	A
		TCF	XMKRUPT		# ITS A X MARK
SOMEKEY		CAF	OCT140		# NOT MARK OR MARK REJECT
		EXTEND
		RAND	NAVKEYIN
		EXTEND
# Page 164		
		BZF	+3		# IF NO BITS
		
		TC	POSTJUMP	# IF DESCENT BITS
		CADR	DESCBITS
		
 +3		TC	ALARM		# NO INBITS IN CHANNEL 16.
		OCT	113
		TC	RESUME
		
XMKRUPT		CAF	ZERO
		TS	RUPTREG1	# SET X MARK STORE INDEX TO ZERO
		CAF	BIT10
		TCF	+4
YMKRUPT		CAF	ONE
		TS	RUPTREG1	# SET Y MARK STORE INDEX TO ONE
		CAF	BIT11
		TS	XYMARK		# SET MARK IDENTIFICATION
		
		MASK	MARKSTAT
		CCS	A
		TCF	+2		# THIS MARK NOT DESIRED
		TCF	VACSTOR		# MARK DESIRED AND MADE-STORE CDUS
		TC	ALARM
		OCT	114
		TC	RESUME		# RENEW REQUEST

VACSTOR		CAF	LOW9		# STORE TIME AND CDUS IN VAC AREA
		MASK	MARKSTAT
		TS	RUPTREG2	# PICK UP VAC ADR
		EXTEND
		DCA	ITEMP1
		INDEX	RUPTREG2
		DXCH	0		# STORE TIME1 AND TIME2 IN VAC
		CA	RUPTREG2
		ADS	RUPTREG1	# INCREMENT CDU STORE ADR BY MARK INDEX
		CA	ITEMP3		# PICK UP CDUY
		INDEX	RUPTREG1
		TS	2
		CA	ITEMP4		# PICK UP CDUZ
		INDEX	RUPTREG1
		TS	4
		CA	ITEMP5		# PICK UP CDUX
		INDEX	RUPTREG1
		TS	6
		
		CAF	BIT13
		AD	XYMARK
		COM
		MASK	MARKSTAT
		AD	XYMARK		# SET MARK MADE BIT IN MARKSTAT=1

# Page 165
		TS	MARKSTAT	# AND SET BIT13 ZERO TO SHOW MARK
		TC	REMARK		# MADE BEFORE A REJECT - GO REMARK.
		
# Page 166
REMARK		CAF	ZERO
		TS	MKDEX		# MKDEX=0
		CAF	BIT11		# LOOK A Y MARK
		MASK	MARKSTAT
		CCS	A
		TC	+3		# Y MARK MADE
		CAF	BIT8		# Y MARK REQUIRED
		TS 	MKDEX
		
		CAF	BIT10		# LOOK AT X MARK
		MASK	MARKSTAT
		CCS	A
		TC	MKJOB
		CAF	BIT7		# X MARK REQUIRED
		ADS	MKDEX
		
MKJOB		CAF	PRIO32
		TC	NOVAC		# ENTER JOB TO PASTE VERB
		EBANK=	AOTAZ
		2CADR	CHANGVB
		
		CA	RUPTREG4
		INDEX	LOCCTR
		TS	MPAC
		TC	RESUME

CHANGVB		CCS	MPAC
		TC	SHOWVB
		CAF	VB21N30E
		TC	SHOWVB	+2
SHOWVB		CAF	VB50
		AD	MPAC
		TS	NVTEMP
		CA	CADRSTOR	# NVSUB CLOBBERS THIS-USE LOC AS TEMP
		TS	LOC
		TC	NVSUB	+3
VB51		OCT	5100
		CA	LOC		# RESTORE CADRSTOR
		TS	CADRSTOR
		TC	ENDOFJOB
		
MKREJ		CAF	BIT14		# IN-FLIGHT OR NON-FLIGHT
		MASK	MARKSTAT
		CCS	A
		TC	REJIN		# IN-FLIGHT
		CAF	BIT10		# NON-FLIGHT, SEE IF X MARK MADE
		MASK	MARKSTAT
		CCS	A
		TC	REJECT2		# XMARK MADE-REJECT IT
		TC	REJALARM	# X MARK NOT MADE-ALARM-NO MARKS TO REJ
		
# Page 167
REJIN		CAF	PRIO3		# WERE IN-FLIGHT MARKS MADE
		MASK	MARKSTAT
		CCS	A
		TC	REJECT		# MARKS MADE-REJECT ONE
REJALARM	TC	ALARM		# NO MARKS MADE TO REJECT-GET MARKS
		OCT	115
		TC	REJECT2
REJECT		CS	BIT13		# SHOW MKREJECT AND SEE MARK MADE
		MASK	MARKSTAT	# SINCE LAST REJECT
		AD	BIT13
		XCH	MARKSTAT
		MASK	BIT13
		CCS	A
		TC	REJECT2		# ANOTHER REJECT
		CS	XYMARK		# MARK MADE SINCE REJECT -- REJECT MARK IN 1D
		XCH	MARKSTAT
		MASK	BIT14		# IN-FLIGHT OR NON-FLIGHT
		CCS	A
		TC	REMARK		# IN-FLIGHT, GO GET MARKS
		CAF	BIT11		# NON-FLIGHT, STILL REQUEST X MARK ONLY
		ADS	MARKSTAT
		TC	REMARK		# GO GET X MARK

OCT3		EQUALS	THREE
OCT34		OCT	00034
OCT140		OCT	00140
V31N43E		OCT	2143
V24N42E		OCT	2442
VB53		OCT	5300
MKDEX		EQUALS	RUPTREG4
DETCODE		EQUALS	XYMARK
BASVAC		EQUALS	Q
VB21N30E	OCT	2130
VB50		OCT	5000

# Page 168
		
RENEWMK		MASK	MARKSTAT
		TS	MARKSTAT
		TCF	REMARK		# GO REQUEST NEW MARK ACTION
		
REJECT2		CS	PRIO3		# ON SECOND REJECT -- DISPLAY VB53 AGAIN
		TCF	RENEWMK
		
SURFREJ		CCS	MARKCNTR	# IF MARK DECREMENT COUNTER
		TCF	+2
		TCF	REJALM		# NO MARKS TO REJECT -- ALARM
		TS	MARKCNTR
		TC	RESUME

