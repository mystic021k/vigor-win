# v_random
# This was blatantly stolen from the TCL FAQ.  It's renamed to prevent
# name clashes.  It seems to have a poor distribution, so I need to find
# a better one.
# Yes, TCL 8.0 has one built in.  No, I don't want to require that.
proc v_random {args} {
    global RNG_seed
    
    set max 259200
    set argcnt [llength $args]
    if { $argcnt < 1 || $argcnt > 2 } {
	error "wrong # args: random limit | seed ?seedval?"
    }
    if ![string compare [lindex $args 0] seed] {
	if { $argcnt == 2 } {
	    set RNG_seed [lindex $args 1]
	} else {
	    set RNG_seed [expr \
                    ([pid]+[clock seconds])%$max]
	}
	return
    }
    if ![info exists RNG_seed] {
	set RNG_seed [expr ([pid]+[clock seconds])%$max]
    }
    set RNG_seed [expr ($RNG_seed*7141+54773) % $max]
    return [expr int(double($RNG_seed)*[lindex $args 0]/$max)]
}

proc vigor_dlg_setup_box {} {
    global vigor_pos
    rotate_vigor_pos

    vigor_animate_bg vigor_rest vigor_wav_0001 vigor_wav_0002 vigor_wav_0001 vigor_wav_0002 vigor_wav_0001 vigor_wav_0002

    toplevel .dlg -class Dialog
    wm title .dlg "Vigor Says"
    wm iconname .dlg "Vigor Says"

    set offset 200
    canvas .dlg.spkframe -width 136 -height 34
    switch -- $vigor_pos {
	"+0+0" {
	    .dlg.spkframe create line 0 34 51 34
	    .dlg.spkframe create line 51 34 34 3
	    .dlg.spkframe create line 34 3 68 34
	    .dlg.spkframe create line 68 34 102 34
	    pack .dlg.spkframe -side top -fill none
	    wm geometry .dlg +0+$offset
	}
	"-0+0" {
	    .dlg.spkframe create line 102 34 51 34
	    .dlg.spkframe create line 51 34 68 3
	    .dlg.spkframe create line 68 3 34 34
	    .dlg.spkframe create line 34 34 0 34
	    pack .dlg.spkframe -side top -fill none
	    wm geometry .dlg -0+$offset
	}
	"+0-0" {
	    .dlg.spkframe create line 0 0 51 0
	    .dlg.spkframe create line 51 0 34 30
	    .dlg.spkframe create line 34 30 68 0
	    .dlg.spkframe create line 68 0 102 0
	    pack .dlg.spkframe -side bottom -fill none
	    wm geometry .dlg +0-$offset
	}
	"default" {
	    .dlg.spkframe create line 102 0 51 0
	    .dlg.spkframe create line 51 0 68 30
	    .dlg.spkframe create line 68 30 34 0
	    .dlg.spkframe create line 34 0 0 0
	    pack .dlg.spkframe -side bottom -fill none
	    wm geometry .dlg -0-$offset
	}
    }
    frame .dlg.btnframe -relief raised -bd 1
    # We create the msgframe, but do not populate it.
    frame .dlg.msgframe
    pack .dlg.msgframe .dlg.btnframe -side top -fill both
}

proc vigor_dlg_setup { text } {
    vigor_dlg_setup_box
    message .dlg.msgframe.msg -width 3i -text $text \
	    -font -Adobe-Times-Medium-R-Normal-*-180-*
    pack .dlg.msgframe.msg -side right -expand 1 -fill both \
	    -padx 3m -pady 3m
}

proc vigor_dlg_block {} {
    set oldFocus [focus]
    grab set .dlg
    focus .dlg
    
    tkwait window .dlg
    vigor_animate_bg vigor_wav_0001 vigor_rest
    focus $oldFocus
}

proc vigor_maybe_confirm { prob text } {
    set val [v_random 100]
    if { $val > $prob } {
	return
    }

    vigor_dlg_setup $text

    button .dlg.btnframe.ok -text OK -command {
	destroy .dlg
    }
    button .dlg.btnframe.cancel -text Cancel -command {
	destroy .dlg
	vigor_comment "If you want to take advantage of the cancel feature, you need to upgrade to the server edition of Vigor.  Contact your sales representative for information."
    }
    switch [ v_random 5 ] {
	"0" {
	    pack .dlg.btnframe.cancel -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
	    pack .dlg.btnframe.ok -side right -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
	}
	default {
	    pack .dlg.btnframe.ok -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
	    pack .dlg.btnframe.cancel -side right -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
	}
    }

    vigor_dlg_block
}

proc vigor_maybe_wizard { prob wizname task0 task1 task2 this_task descr } {
    global vigor_wizard_descr vigor_wizard_task1 vigor_wizard_task2

#    viMsg 1 "$prob chance of winning"
    set val [v_random 100]
    if { $val > $prob } {
	return
    }

#    viMsg 1 "foo"
    # I have no idea how to create closures in Tcl, so we fake it with globals
    set vigor_wizard_descr $descr
    set vigor_wizard_task1 $task1
    set vigor_wizard_task2 $task2
    vigor_dlg_setup "It looks like you are $wizname.  Which Vigor Whizzer would you like to use to help you with this?"
#    viMsg 1 "mane"

    button .dlg.btnframe.task0 -text $task0 -command {
	destroy .dlg
	vigor_comment $vigor_wizard_descr
    }
    button .dlg.btnframe.task1 -text $task1 -command {
	destroy .dlg
	vigor_comment "If you want to $vigor_wizard_task1 then you need to invoke the \"$vigor_wizard_task1\" Whizzer."
    }
    button .dlg.btnframe.task2 -text $task2 -command {
	destroy .dlg
	vigor_comment "If you want to $vigor_wizard_task2 then you need to invoke the \"$vigor_wizard_task2\" Whizzer."
    }
    button .dlg.btnframe.cancel -text "Cancel" -command {
	destroy .dlg
	vigor_comment "If you want to take advantage of the cancel feature, you need to upgrade to the server edition of Vigor.  Contact your sales representative for information."
	vigor_comment $vigor_wizard_descr
    }
#    viMsg 1 "padme"
    pack .dlg.btnframe.task0 -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
    pack .dlg.btnframe.task1 -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
    pack .dlg.btnframe.task2 -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
    # FIXME This does not seem to work right.  The "Cancel" button is always
    # on the right.  I have little concern over such things, though, when this
    # is now the fourth month of a weekend hack.
    switch [ v_random 5 ] {
	"0" {
	    pack .dlg.btnframe.cancel -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
	}
	default {
	    pack .dlg.btnframe.cancel -side right -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m
	}
    }

#    viMsg 1 "hum"
    vigor_dlg_block
}

proc vigor_confirm { text } {
    vigor_maybe_confirm 100 $text
}

proc vigor_wizard { wizname task0 task1 task2 this_task descr } {
    # FIXME I know there's a better way.  I just don't remember it now,
    # and really want to just finish this and get some sleep.
    vigor_maybe_wizard 100 $wizname $task0 $task1 $task2 $this_task $descr
}

proc vigor_hint { text } {
    vigor_confirm $text
}

proc vigor_comment { text } {
    vigor_dlg_setup $text

    button .dlg.btnframe.ok -text OK -command {
	destroy .dlg
    }
    pack .dlg.btnframe.ok -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m

    vigor_dlg_block
}

proc vigor_eula {} {
    vigor_dlg_setup_box

    text .dlg.msgframe.text -relief raised -bd 2 -yscrollcommand {
	# FIXME For some reason, if we rotate_vigor_pos here,
	# it goes hysterical.  This even happens if a second's worth
	# of rotate's are eaten by the lag code.  Does anybody know why?
	# rotate_vigor_pos
	.dlg.msgframe.text yview 0
	# This comment eats the output of yscrollcommand }
    # text .dlg.msgframe.text -relief raised -bd 2 -yscrollcommand ".dlg.msgframe.scroll set"
    scrollbar .dlg.msgframe.scroll
    # -command ".dlg.msgframe.text yview"
    bind .dlg.msgframe.scroll <Enter> { rotate_vigor_pos }
    pack .dlg.msgframe.scroll -side right -fill y
    pack .dlg.msgframe.text -side left

    .dlg.msgframe.text insert end "END-USER LICENSE AGREEMENT\n\n"

    .dlg.msgframe.text insert end "IMPORTANT-READ CAREFULLY: "
    .dlg.msgframe.text insert end "This End-User License Agreement (\"EULA\") "
    .dlg.msgframe.text insert end "is NOT a legal agreement between you "
    .dlg.msgframe.text insert end "(either an individual or a single entity, "
    .dlg.msgframe.text insert end "so take that Fare) and some other unnamed "
    .dlg.msgframe.text insert end "entity for whatever product we feel like "
    .dlg.msgframe.text insert end "it applies to.  This includes media, "
    .dlg.msgframe.text insert end "printed materials, online or electronic "
    .dlg.msgframe.text insert end "documentation, products purchased because "
    .dlg.msgframe.text insert end "of or used with the product, or anything "
    .dlg.msgframe.text insert end "that happens to be nearby (\"SOFTWARE "
    .dlg.msgframe.text insert end "PRODUCT\").  By installing, copying, "
    .dlg.msgframe.text insert end "downloading, accessing, using, thinking "
    .dlg.msgframe.text insert end "about, hearing of, or being in the same "
    .dlg.msgframe.text insert end "universe as the SOFTWARE PRODUCT, you "
    .dlg.msgframe.text insert end "agree to be bound by the terms of this "
    .dlg.msgframe.text insert end "EULA.  If you do not agree to the terms "
    .dlg.msgframe.text insert end "of this EULA, you can try to get your "
    .dlg.msgframe.text insert end "money back, but let's face it, that ain't "
    .dlg.msgframe.text insert end "gonna happen anyway, so why bother?\n\n"

    .dlg.msgframe.text insert end "SOFTWARE PRODUCT LICENSE\n\n"
    .dlg.msgframe.text insert end "The SOFTWARE PRODUCT is protected by "
    .dlg.msgframe.text insert end "copyright laws, international copyright "
    .dlg.msgframe.text insert end "treaties, several vaguely defined IPO "
    .dlg.msgframe.text insert end "laws and treaties that are easily clouded "
    .dlg.msgframe.text insert end "in a court of law, and a guy named "
    .dlg.msgframe.text insert end "Bubba with a shotgun.\n\n"

    # FIXME This paragraph needs some work.
    .dlg.msgframe.text insert end "1. GRANT OF LICENSE. This EULA grants you "
    .dlg.msgframe.text insert end "the following rights:\n"
    .dlg.msgframe.text insert end "* The right to do whatever the publisher "
    .dlg.msgframe.text insert end "says.\n"
    .dlg.msgframe.text insert end "* Nothing else.\n\n"

    # FIXME This paragraph needs some work too.
    .dlg.msgframe.text insert end "2. RESPONSIBILITIES OF THE USER.  By "
    .dlg.msgframe.text insert end "agreeing to this license, you agree to "
    .dlg.msgframe.text insert end "the following:\n"
    # This is where it cuts off on a 1024x768 display.
    .dlg.msgframe.text insert end "* Give the provider your firstborn son, "
    .dlg.msgframe.text insert end "should the provider deem such is "
    .dlg.msgframe.text insert end "necessary and valuable.\n"
    .dlg.msgframe.text insert end "* Become Bill's towel boy for life.\n"
    .dlg.msgframe.text insert end "* Whatever else we think of later on, "
    .dlg.msgframe.text insert end "you already agreed to from the shrink-wrap "
    .dlg.msgframe.text insert end "license anyway\n\n"

    .dlg.msgframe.text insert end "THIS TECHNOLOGY IS NOT FAULT TOLERANT AND "
    .dlg.msgframe.text insert end "IS NOT DESIGNED, MANUFACTURED, OR INTENDED "
    .dlg.msgframe.text insert end "FOR USE OR RESALE AS ON-LINE CONTROL "
    .dlg.msgframe.text insert end "EQUIPMENT IN HAZARDOUS ENVIRONMENTS "
    .dlg.msgframe.text insert end "REQUIRING FAIL-SAFE PERFORMANCE, SUCH AS "
    .dlg.msgframe.text insert end "THE OPERATION OF NUCLEAR FACILITIES, "
    .dlg.msgframe.text insert end "AIRCRAFT NAVIGATION OR COMMUNICATIONS "
    .dlg.msgframe.text insert end "SYSTEMS, AIR TRAFFIC CONTROL, DIRECT LIFE "
    .dlg.msgframe.text insert end "SUPPORT MACHINES, WEAPONS SYSTEMS, "
    .dlg.msgframe.text insert end "INTERNET SERVICE PROVIDERS, TECHNICAL "
    .dlg.msgframe.text insert end "SUPPORT LINES, NETWORK OPERATIONS CENTERS, "
    .dlg.msgframe.text insert end "IN WHICH THE FAILURE OF THE TECHNOLOGY "
    .dlg.msgframe.text insert end "OR NORMAL DAY-TO-DAY OPERATIONS COULD "
    .dlg.msgframe.text insert end "LEAD DIRECTLY TO DEATH, PERSONAL INJURY, "
    .dlg.msgframe.text insert end "OR SEVERE PHYSICAL OR ENVIRONMENTAL "
    .dlg.msgframe.text insert end "DAMAGE.\n\n"

    .dlg.msgframe.text insert end "LIMITED WARRANTY\n"
    .dlg.msgframe.text insert end "You have no warranty rights under this "
    .dlg.msgframe.text insert end "agreement whatsoever, expressed or "
    .dlg.msgframe.text insert end "implied.  We don't want to hear from you "
    .dlg.msgframe.text insert end "if you've got any problems.  We don't "
    .dlg.msgframe.text insert end "give you a number to call for just that "
    .dlg.msgframe.text insert end "reason.  You've got to call the poor "
    .dlg.msgframe.text insert end "schmuck who sold this to you; they have to "
    .dlg.msgframe.text insert end "support you themselves, and we won't help "
    .dlg.msgframe.text insert end "them.  We don't care.  We don't have "
    .dlg.msgframe.text insert end "to.\n\n"

    .dlg.msgframe.text insert end "If you're reading this because it's being "
    .dlg.msgframe.text insert end "displayed as part of the Vigor startup "
    .dlg.msgframe.text insert end "sequence, then please let me know by email "
    .dlg.msgframe.text insert end "at joelh@gnu.org.  This dialog box is "
    .dlg.msgframe.text insert end "intended to be long enough so that it "
    .dlg.msgframe.text insert end "won't be displayed on the screen.\n\n"
    .dlg.msgframe.text insert end "Exception: I know that you can click in "
    .dlg.msgframe.text insert end "the text box, and then hold down the "
    .dlg.msgframe.text insert end "down arrow key, and thus bypass the scroll "
    .dlg.msgframe.text insert end "overrides.  I'll figure out a way to keep "
    .dlg.msgframe.text insert end "that from happening later."

    button .dlg.btnframe.ok -text "I Accept" -command {
	destroy .dlg
    }
    pack .dlg.btnframe.ok -side left -expand 1 -padx 3m -pady 3m -ipadx 2m -ipady 1m

    vigor_dlg_block
}

# For some reason, the window isn't moving every time it should.  I haven't
# yet figured out why that is.  I think there may be an issue with the
# while test, but I'm not sure.
proc rotate_vigor_pos {} {
    global vigor_pos vigor_holdit
    if { $vigor_holdit } {
	return $vigor_pos
    }
    # This is a lag to keep vigor from going hysterical if it gets constant
    # rotate calls.  (This happened at one point in the EULA, and I'm not
    # sure why.)
    set vigor_holdit 1
    after 500 { set vigor_holdit 0 }
    set old_vigor_pos $vigor_pos
    # switch -- $vigor_pos {}
    while { $old_vigor_pos == $vigor_pos } {
	switch [ v_random 4 ] {
	    "0" { set vigor_pos "-0+0" }
	    "1" { set vigor_pos "-0-0" }
	    "2" { set vigor_pos "+0-0" }
	    default { set vigor_pos "+0+0" }
	}
    }
    wm geometry . $vigor_pos
    if { ! [ winfo exists .dlg ] } {
	return $vigor_pos
    }

    # We have a dialog that should be moved.  We recreate a fair bit of it.
    set offset 200
    pack unpack .dlg.btnframe
    pack unpack .dlg.msgframe
    destroy .dlg.spkframe
    canvas .dlg.spkframe -width 136 -height 34
    switch -- $vigor_pos {
	"+0+0" {
	    .dlg.spkframe create line 0 34 51 34
	    .dlg.spkframe create line 51 34 34 3
	    .dlg.spkframe create line 34 3 68 34
	    .dlg.spkframe create line 68 34 102 34
	    pack .dlg.spkframe -side top -fill none
	    wm geometry .dlg +0+$offset
	}
	"-0+0" {
	    .dlg.spkframe create line 102 34 51 34
	    .dlg.spkframe create line 51 34 68 3
	    .dlg.spkframe create line 68 3 34 34
	    .dlg.spkframe create line 34 34 0 34
	    pack .dlg.spkframe -side top -fill none
	    wm geometry .dlg -0+$offset
	}
	"+0-0" {
	    .dlg.spkframe create line 0 0 51 0
	    .dlg.spkframe create line 51 0 34 30
	    .dlg.spkframe create line 34 30 68 0
	    .dlg.spkframe create line 68 0 102 0
	    pack .dlg.spkframe -side bottom -fill none
	    wm geometry .dlg +0-$offset
	}
	"default" {
	    .dlg.spkframe create line 102 0 51 0
	    .dlg.spkframe create line 51 0 68 30
	    .dlg.spkframe create line 68 30 34 0
	    .dlg.spkframe create line 34 0 0 0
	    pack .dlg.spkframe -side bottom -fill none
	    wm geometry .dlg -0-$offset
	}
    }
    pack .dlg.msgframe .dlg.btnframe -side top -fill both
    return $vigor_pos
}

proc change_xbm { newname } {
    global xbmid
    .vigor delete $xbmid
    set xbmid [ .vigor create bitmap 10p 10p -bitmap $newname -anchor nw ]
}

proc vigor_animate args {
    foreach frame $args {
	change_xbm $frame
	update
	after 250
    }
}

proc vigor_cp {} {
    # The canvas is a dummy to block vigor_randomness
    canvas .dlg
    set i [ vigor_animate_bg vigor_rest v2cp0004 v2cp0003 v2cp0002 v2cp0001 v2cp0000 v2cp0000 v2cp0001 v2cp0002 v2cp0003 v2cp0004 vigor_rest ]
    after [ expr 250 * $i ] { destroy .dlg }
}

# vigor_randomness
# Called once every time input is requested, and once every five seconds.
# Should do random annoying things.
#
# Taunts came from Noah Friedman's silly-mail.el.
# They have come from Noah Friedman, Jamie Zawinski, Jim Blandy,
# Thomas Bushnell, Roland McGrath, and a cast of dozens.
# Some of these were originally taunts about Emacs or X Windows.
proc vigor_randomness {} {
    if { [ winfo exists .dlg ] } {
	return
    }
    switch -glob [v_random 4000] {
	0 { vigor_confirm "Are you sure you wish to have a spontanious pohn fahr?" }
	1 { vigor_comment "Don't cry --- it won't help." }
	2 { vigor_comment "Don't try this at home, kids!" }
	3 { vigor_comment "Or perhaps you'd prefer Russian Routlette, after all?" }
	4 { vigor_comment "You'll understand when you're older, dear." }
	5 { vigor_comment "Want some rye?  'Course ya do!" }
	6 { vigor_comment "Your blood pressure just went up a point." }
	7 { vigor_comment "Your blood pressure just went up a point." }
	8 { vigor_comment "Your blood pressure just went up a point." }
	100 { vigor_comment "Vigor: A compelling argument for pencil and paper." }
	101 { vigor_comment "Vigor: An inspiring example of form following function... to Hell." }
	102 { vigor_comment "Vigor: Because Hell was full." }
	103 { vigor_comment "Vigor is freely redistributable; void where prohibited by law." }
	104 { vigor_comment "If SIGINT doesn't work, try a tranquilizer." }
	105 { vigor_comment "It's all fun and games, until somebody tries to edit a file." }
	106 { vigor_comment "Lovecraft was an optimist." }
	107 { vigor_comment "Vigor: More than just a pretty face!" }
	108 { vigor_comment "Vigor: The only text editor known to get indigestion." }
	109 { vigor_comment "Vigor: Where editing text is like playing Paganini on a glass harmonica." }
	110 { vigor_comment "Vigor: All the problems and twice the bugs." }
	111 { vigor_comment "Vigor: Dissatisfaction guaranteed." }
	112 { vigor_comment "Vigor: Don't get frustrated without it." }
	113 { vigor_comment "Vigor: Even not doing anything would have been better than nothing." }
	114 { vigor_comment "Vigor: Even your dog won't like it." }
	115 { vigor_comment "Vigor: Form follows malfunction." }
	116 { vigor_comment "Vigor: It could be worse, but it'll take time." }
	117 { vigor_comment "Vigor: It could happen to you." }
	118 { vigor_comment "Vigor: Let it get in *your* way." }
	119 { vigor_comment "Vigor: Live the nightmare." }
	120 { vigor_comment "Vigor: Putting new limits on productivity." }
	121 { vigor_comment "Vigor: Some voids are better left unfilled." }
	122 { vigor_comment "Vigor: The problem for your problem." }
	150 { vigor_comment "Vi: It was hard to write; it should be hard to use." }
	151 { vigor_comment "VI VI VI: The editor of the beast" }
	152 { vigor_comment "Vigor: Putting the ass in assist" }
	200 { vigor_comment "X Windows: A mistake carried out to perfection." }
	201 { vigor_comment "X Windows: A terminal disease." }
	"50?" { vigor_cp }
	"51?" { vigor_cp }
	"52?" { vigor_cp }
    }
}

# This should be invoked once.  It causes itself to be invoked every 5000 ms.
proc vigor_handle_randomness {} {
    vigor_randomness
    after 5000 { vigor_handle_randomness }
}

proc vigor_animate_bg args {
    set i 0
    foreach frame $args {
	after [ expr 250 * $i ] [ list change_xbm $frame ]
	set i [ expr $i + 1 ]
    }
    return $i
}

proc vigor_init {} {
    global vigor_pos xbmid vigor_holdit
    wm title . Vigor
    wm iconname . Vigor
    canvas .vigor -width 128p -height 128p -background white
    set xbmid [ .vigor create bitmap 10p 10p -bitmap vigor_empty -anchor nw ]
    pack .vigor
    bind . <Enter> { rotate_vigor_pos }
    wm protocol . WM_DELETE_WINDOW {
	vigor_confirm "Are you sure you don't want to close the Vigor assistant?"
	vigor_comment "Assistant not closed."
    }
    set vigor_pos -0+0
    set vigor_holdit 0
    rotate_vigor_pos
    vigor_animate vigor_empty vigor_ent_0001 vigor_ent_0002 vigor_ent_0003 vigor_ent_0004 vigor_rest
    vigor_eula
    vigor_comment "Hi there!  I'm the Vigor Assistant!  If you have any questions about VI, just ask me!"
    vigor_comment "If you haven't used Vigor before, be sure to remember that the Vigor Assistant is here to help you!"
    vigor_handle_randomness
}
