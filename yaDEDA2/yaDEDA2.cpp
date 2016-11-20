// -*- C++ -*- generated by wxGlade 0.6.3 on Thu Mar 12 19:52:16 2009
/*
 * Copyright 2009 Ronald S. Burkey <info@sandroid.org>
 *
 * This file is part of yaAGC.
 *
 * yaAGC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * yaAGC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with yaAGC; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Filename:	yaDEDA2.cpp
 * Contact:	Ron Burkey <info@sandroid.org>
 * Reference:	http://www.ibiblio.org/apollo/index.html
 * Mods:	2009-03-12 RSB	Began.
 * 		2009-03-14 RSB	Added a workaround for a funky socket
 *				error which was showing up in Windows
 *				for the yaTelemetry program (which uses
 *				identical socket-management code) but
 *				which wasn't showing up here for some
 *				reason ... yet.  Elminated some
 *				compiler warnings due to now-deprecated
 *				initializers for char *.
 *		2009-03-19 RSB	Changed the key-releases for ENTR and
 *				READ OUT so that they always happen
 *				after the data-shifting.  This is an
 *				attempt to work around the weird behavior
 *				I've been seeing with the DEDA essentially
 *				freezing up.
 *
 * The yaDSKY2 program is intended to be a completely identical drop-in
 * replacement for the yaDEDA program as it exists at 2009-03-12.
 * yaDEDA works well, but it suffers from the basic problem that GTK+
 * support for Mac OS X (and even to some extent for Win32) is not
 * where I want it to be.  Furthermore, newer GUI programs for yaAGC
 * are based on wxWidgets, where I have been having better luck with
 * Win32/Mac support.  So in other words, yaDEDA2 is a wxWidgets port
 * of the GTK+ based yaDEDA program.
 */

#include "yaDEDA2.h"

#define VER(x) #x

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

using namespace std;

#include "../yaAGC/yaAGC.h"
#include "../yaAGC/agc_engine.h"

#ifdef WIN32
clock_t
times (struct tms *p)
  {
    return (GetTickCount ());
  }
#endif

// Interval (ms.) at which the timer fires for servicing the socket.
#define PULSE_INTERVAL 100

static MainFrame *MainWindow;
static char DefaultHostname[] = "localhost";
static char *Hostname = DefaultHostname;
static char NonDefaultHostname[129];
#ifdef WIN32
static int StartupDelay = 500;
#else
static int StartupDelay = 0;
#endif
extern int Portnum;
static int ServerSocket = -1;

// Names of various graphics files.

static const char SevenSeg0Xpm[] = "7Seg-0.xpm", SevenSeg1Xpm[] = "7Seg-1.xpm",
    SevenSeg2Xpm[] = "7Seg-2.xpm", SevenSeg3Xpm[] = "7Seg-3.xpm",
    SevenSeg4Xpm[] = "7Seg-4.xpm", SevenSeg5Xpm[] = "7Seg-5.xpm",
    SevenSeg6Xpm[] = "7Seg-6.xpm", SevenSeg7Xpm[] = "7Seg-7.xpm",
    SevenSeg8Xpm[] = "7Seg-8.xpm", SevenSeg9Xpm[] = "7Seg-9.xpm",
    SevenSegOffXpm[] = "7SegOff.xpm", PlusOnXpm[] = "PlusOn.xpm", MinusOnXpm[] =
	"MinusOn.xpm", PlusMinusOffXpm[] = "PlusMinusOff.xpm";

static const char *DigitFilenames[16] =
  { SevenSeg0Xpm, SevenSeg1Xpm, SevenSeg2Xpm, SevenSeg3Xpm, SevenSeg4Xpm,
      SevenSeg5Xpm, SevenSeg6Xpm, SevenSeg7Xpm, SevenSeg8Xpm, SevenSeg9Xpm,
      SevenSegOffXpm, SevenSegOffXpm, SevenSegOffXpm, SevenSegOffXpm,
      SevenSegOffXpm, SevenSegOffXpm };

static const char *SignFilenames[16] =
  { PlusOnXpm, MinusOnXpm, PlusMinusOffXpm, PlusMinusOffXpm, PlusMinusOffXpm,
      PlusMinusOffXpm, PlusMinusOffXpm, PlusMinusOffXpm, PlusMinusOffXpm,
      PlusMinusOffXpm, PlusMinusOffXpm, PlusMinusOffXpm, PlusMinusOffXpm,
      PlusMinusOffXpm, PlusMinusOffXpm, PlusMinusOffXpm };

static const char *OprErrFilenames[2] =
  { "OprErrOff.xpm", "OprErrOn.xpm" };

// begin wxGlade: ::extracode
// end wxGlade

MainFrame::MainFrame (wxWindow* parent, int id, const wxString& title,
		      const wxPoint& pos, const wxSize& size, long style) :
    wxFrame (parent, id, title, pos, size,
	     wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxSYSTEM_MENU)
{
  HalfSize = false;
  OprErr = 0;
  NumInPacket = 0;
  NumOutPacket = 10;
  LastReadout = 0;
  ShiftingOut = 0;
  ShiftBufferSize = 0;
  ShiftBufferPtr = 0;
  WhenInPacket = 0;
  WhenOutPacket = 0;

  // begin wxGlade: MainFrame::MainFrame
  bitmap_10 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderHorizontal158.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_5 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderVertical48.xpm"), wxBITMAP_TYPE_ANY));
  DigitTop1 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitTop2 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitTop3 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_5_copy = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderVertical48.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_11 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderHorizontal158.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_12 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderHorizontal88.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_5_copy_1 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderVertical48.xpm"), wxBITMAP_TYPE_ANY));
  IndicatorOprErr = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("OprErrOff.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_20 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderVertical48.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_12_copy = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderHorizontal88.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_13 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderHorizontal290.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_7 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderVertical48.xpm"), wxBITMAP_TYPE_ANY));
  Sign = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("PlusMinusOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitBottom1 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitBottom2 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitBottom3 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitBottom4 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  DigitBottom5 = new wxStaticBitmap (
      this, wxID_ANY, wxBitmap (wxT ("7SegOff.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_7_copy = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderVertical48.xpm"), wxBITMAP_TYPE_ANY));
  bitmap_14 = new wxStaticBitmap (
      this, wxID_ANY,
      wxBitmap (wxT ("BorderHorizontal290.xpm"), wxBITMAP_TYPE_ANY));
  KeyPlus = new wxBitmapButton (
      this, ID_PLUS, wxBitmap (wxT ("PlusUp.xpm"), wxBITMAP_TYPE_ANY));
  Key7 = new wxBitmapButton (this, ID_7,
			     wxBitmap (wxT ("7Up.xpm"), wxBITMAP_TYPE_ANY));
  Key8 = new wxBitmapButton (this, ID_8,
			     wxBitmap (wxT ("8Up.xpm"), wxBITMAP_TYPE_ANY));
  Key9 = new wxBitmapButton (this, ID_9,
			     wxBitmap (wxT ("9Up.xpm"), wxBITMAP_TYPE_ANY));
  KeyClr = new wxBitmapButton (this, ID_CLR,
			       wxBitmap (wxT ("ClrUp.xpm"), wxBITMAP_TYPE_ANY));
  KeyMinus = new wxBitmapButton (
      this, ID_MINUS, wxBitmap (wxT ("MinusUp.xpm"), wxBITMAP_TYPE_ANY));
  Key4 = new wxBitmapButton (this, ID_4,
			     wxBitmap (wxT ("4Up.xpm"), wxBITMAP_TYPE_ANY));
  Key5 = new wxBitmapButton (this, ID_5,
			     wxBitmap (wxT ("5Up.xpm"), wxBITMAP_TYPE_ANY));
  Key6 = new wxBitmapButton (this, ID_6,
			     wxBitmap (wxT ("6Up.xpm"), wxBITMAP_TYPE_ANY));
  KeyReadOut = new wxBitmapButton (
      this, ID_READOUT, wxBitmap (wxT ("ReadOutUp.xpm"), wxBITMAP_TYPE_ANY));
  Key0 = new wxBitmapButton (this, ID_0,
			     wxBitmap (wxT ("0Up.xpm"), wxBITMAP_TYPE_ANY));
  Key1 = new wxBitmapButton (this, ID_1,
			     wxBitmap (wxT ("1Up.xpm"), wxBITMAP_TYPE_ANY));
  Key2 = new wxBitmapButton (this, ID_2,
			     wxBitmap (wxT ("2Up.xpm"), wxBITMAP_TYPE_ANY));
  Key3 = new wxBitmapButton (this, ID_3,
			     wxBitmap (wxT ("3Up.xpm"), wxBITMAP_TYPE_ANY));
  KeyEntr = new wxBitmapButton (
      this, ID_ENTR, wxBitmap (wxT ("EntrUp.xpm"), wxBITMAP_TYPE_ANY));
  KeyHold = new wxBitmapButton (
      this, ID_HOLD, wxBitmap (wxT ("HoldUp.xpm"), wxBITMAP_TYPE_ANY));

  set_properties ();
  do_layout ();
  // end wxGlade
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
// begin wxGlade: MainFrame::event_table
EVT_BUTTON(ID_PLUS, MainFrame::on_KeyPlus_pressed)
EVT_BUTTON(ID_7, MainFrame::on_Key7_pressed)
EVT_BUTTON(ID_8, MainFrame::on_Key8_pressed)
EVT_BUTTON(ID_9, MainFrame::on_Key9_pressed)
EVT_BUTTON(ID_CLR, MainFrame::on_KeyClr_pressed)
EVT_BUTTON(ID_MINUS, MainFrame::on_KeyMinus_pressed)
EVT_BUTTON(ID_4, MainFrame::on_Key4_pressed)
EVT_BUTTON(ID_5, MainFrame::on_Key5_pressed)
EVT_BUTTON(ID_6, MainFrame::on_Key6_pressed)
EVT_BUTTON(ID_READOUT, MainFrame::on_KeyReadOut_pressed)
EVT_BUTTON(ID_0, MainFrame::on_Key0_pressed)
EVT_BUTTON(ID_1, MainFrame::on_Key1_pressed)
EVT_BUTTON(ID_2, MainFrame::on_Key2_pressed)
EVT_BUTTON(ID_3, MainFrame::on_Key3_pressed)
EVT_BUTTON(ID_ENTR, MainFrame::on_KeyEntr_pressed)
EVT_BUTTON(ID_HOLD, MainFrame::on_KeyHold_pressed)
// end wxGlade
END_EVENT_TABLE();

void
MainFrame::on_KeyPlus_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket != 3, 0);
}

void
MainFrame::on_Key7_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 7);
}

void
MainFrame::on_Key8_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket < 4 || NumOutPacket > 9, 8);
}

void
MainFrame::on_Key9_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket < 4 || NumOutPacket > 9, 9);
}

void
MainFrame::on_KeyClr_pressed (wxCommandEvent &event)
{
  // Pressed.
  OperatorError (0);
  OutputData (05, 0757020);		// active 0
  NumOutPacket = 0;
  ClearTheDisplay ();
  LastReadout = 0;

  wxMilliSleep (200);

  // Released.
  OutputData (05, 0777020);		// inactive 1
}

void
MainFrame::on_KeyMinus_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket != 3, 1);
}

void
MainFrame::on_Key4_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 4);
}

void
MainFrame::on_Key5_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 5);
}

void
MainFrame::on_Key6_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 6);
}

void
MainFrame::on_KeyReadOut_pressed (wxCommandEvent &event)
{
  // Pressed.
  if (NumOutPacket == 10 && LastReadout == 1)
    NumOutPacket = 3;
  OperatorError (NumOutPacket != 3);
  if (OprErr)
    return;
  OutputData (05, 0775002);		// active 0
  ReadyForShift ();
  LastReadout = 1;

  //wxMilliSleep (200);

  // Released.
  //OutputData (05, 0777002);		// inactive 1
}

void
MainFrame::on_Key0_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 0);
}

void
MainFrame::on_Key1_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 1);
}

void
MainFrame::on_Key2_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 2);
}

void
MainFrame::on_Key3_pressed (wxCommandEvent &event)
{
  ProcessDigit (NumOutPacket == 3 || NumOutPacket > 9, 3);
}

void
MainFrame::on_KeyEntr_pressed (wxCommandEvent &event)
{
  // Press
  OperatorError (NumOutPacket != 9);
  if (OprErr)
    return;
  OutputData (05, 0773004);		// active 0.
  ReadyForShift ();
  LastReadout = 0;

  //wxMilliSleep (200);

  // Release
  //OutputData (05, 0777004);		// inactive 1
}

void
MainFrame::on_KeyHold_pressed (wxCommandEvent &event)
{
  // Pressed.
  OperatorError (LastReadout != 1);
  if (OprErr)
    return;
  OutputData (05, 0767010);		// active 0

  wxMilliSleep (200);

  // Released.
  OutputData (05, 0777010);		// inactive 1
}

// wxGlade: add MainFrame event handlers

void
MainFrame::set_properties ()
{
  // begin wxGlade: MainFrame::set_properties
  SetTitle (wxT ("yaDEDA2 by Ron Burkey"));
  wxIcon _icon;
  _icon.CopyFromBitmap (wxBitmap (wxT ("ApolloPatch2.png"), wxBITMAP_TYPE_ANY));
  SetIcon (_icon);
  bitmap_5->SetMinSize (wxSize (4, 45));
  bitmap_5_copy->SetMinSize (wxSize (4, 45));
  bitmap_12->SetMinSize (wxSize (92, 4));
  bitmap_5_copy_1->SetMinSize (wxSize (4, 40));
  bitmap_20->SetMinSize (wxSize (4, 40));
  bitmap_12_copy->SetMinSize (wxSize (92, 4));
  bitmap_7->SetMinSize (wxSize (4, 45));
  bitmap_7_copy->SetMinSize (wxSize (4, 45));
  KeyPlus->SetMinSize (wxSize (75, 75));
  Key7->SetMinSize (wxSize (75, 75));
  Key8->SetMinSize (wxSize (75, 75));
  Key9->SetMinSize (wxSize (75, 75));
  KeyClr->SetMinSize (wxSize (75, 75));
  KeyMinus->SetMinSize (wxSize (75, 75));
  Key4->SetMinSize (wxSize (75, 75));
  Key5->SetMinSize (wxSize (75, 75));
  Key6->SetMinSize (wxSize (75, 75));
  KeyReadOut->SetMinSize (wxSize (75, 75));
  Key0->SetMinSize (wxSize (75, 75));
  Key1->SetMinSize (wxSize (75, 75));
  Key2->SetMinSize (wxSize (75, 75));
  Key3->SetMinSize (wxSize (75, 75));
  KeyEntr->SetMinSize (wxSize (75, 75));
  KeyHold->SetMinSize (wxSize (75, 75));
  // end wxGlade
}

void
MainFrame::do_layout ()
{
  // begin wxGlade: MainFrame::do_layout
  wxBoxSizer* sizer_1 = new wxBoxSizer (wxVERTICAL);
  wxBoxSizer* sizer_2 = new wxBoxSizer (wxHORIZONTAL);
  wxGridSizer* grid_sizer_1 = new wxGridSizer (4, 5, 5, 5);
  wxBoxSizer* sizer_3 = new wxBoxSizer (wxHORIZONTAL);
  wxBoxSizer* sizer_8 = new wxBoxSizer (wxVERTICAL);
  wxBoxSizer* sizer_9 = new wxBoxSizer (wxHORIZONTAL);
  wxBoxSizer* sizer_7 = new wxBoxSizer (wxVERTICAL);
  wxBoxSizer* sizer_10 = new wxBoxSizer (wxHORIZONTAL);
  wxBoxSizer* sizer_4 = new wxBoxSizer (wxHORIZONTAL);
  wxBoxSizer* sizer_5 = new wxBoxSizer (wxVERTICAL);
  wxBoxSizer* sizer_6 = new wxBoxSizer (wxHORIZONTAL);
  sizer_1->Add (20, 10, 0, wxEXPAND, 0);
  sizer_4->Add (20, 20, 1, 0, 0);
  sizer_5->Add (bitmap_10, 0, 0, 0);
  sizer_6->Add (bitmap_5, 0, 0, 0);
  sizer_6->Add (DigitTop1, 0, 0, 0);
  sizer_6->Add (DigitTop2, 0, 0, 0);
  sizer_6->Add (DigitTop3, 0, 0, 0);
  sizer_6->Add (bitmap_5_copy, 0, 0, 0);
  sizer_5->Add (sizer_6, 0, 0, 0);
  sizer_5->Add (bitmap_11, 0, 0, 0);
  sizer_4->Add (sizer_5, 0, 0, 0);
  sizer_4->Add (20, 20, 1, 0, 0);
  sizer_1->Add (sizer_4, 0, wxEXPAND, 0);
  sizer_1->Add (20, 30, 0, wxEXPAND, 0);
  sizer_3->Add (1, 20, 1, 0, 0);
  sizer_7->Add (bitmap_12, 0, 0, 0);
  sizer_10->Add (bitmap_5_copy_1, 0, 0, 0);
  sizer_10->Add (IndicatorOprErr, 0,
		 wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  sizer_10->Add (bitmap_20, 0, 0, 0);
  sizer_7->Add (sizer_10, 0, 0, 0);
  sizer_7->Add (bitmap_12_copy, 0, 0, 0);
  sizer_3->Add (sizer_7, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
		0);
  sizer_3->Add (20, 20, 5, 0, 0);
  sizer_8->Add (bitmap_13, 0, 0, 0);
  sizer_9->Add (bitmap_7, 0, wxEXPAND, 0);
  sizer_9->Add (Sign, 0, 0, 0);
  sizer_9->Add (DigitBottom1, 0, 0, 0);
  sizer_9->Add (DigitBottom2, 0, 0, 0);
  sizer_9->Add (DigitBottom3, 0, 0, 0);
  sizer_9->Add (DigitBottom4, 0, 0, 0);
  sizer_9->Add (DigitBottom5, 0, 0, 0);
  sizer_9->Add (bitmap_7_copy, 0, wxEXPAND, 0);
  sizer_8->Add (sizer_9, 1, wxEXPAND, 0);
  sizer_8->Add (bitmap_14, 0, 0, 0);
  sizer_3->Add (sizer_8, 0, 0, 0);
  sizer_3->Add (1, 20, 1, 0, 0);
  sizer_1->Add (sizer_3, 0, wxEXPAND, 0);
  sizer_1->Add (20, 15, 0, wxEXPAND, 0);
  sizer_2->Add (10, 20, 1, wxEXPAND, 0);
  grid_sizer_1->Add (KeyPlus, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key7, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key8, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key9, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (KeyClr, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (KeyMinus, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key4, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key5, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key6, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (KeyReadOut, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key0, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key1, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key2, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (Key3, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (KeyEntr, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (
      20, 20, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
      0);
  grid_sizer_1->Add (
      20, 20, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
      0);
  grid_sizer_1->Add (KeyHold, 0,
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
  grid_sizer_1->Add (
      20, 20, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
      0);
  grid_sizer_1->Add (
      20, 20, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
      0);
  sizer_2->Add (grid_sizer_1, 16, wxEXPAND, 0);
  sizer_2->Add (10, 20, 1, wxEXPAND, 0);
  sizer_1->Add (sizer_2, 0, wxEXPAND, 0);
  sizer_1->Add (20, 10, 0, wxEXPAND, 0);
  SetSizer (sizer_1);
  sizer_1->Fit (this);
  Layout ();
  // end wxGlade
}

class yaDedaAppClass : public wxApp
{
public:
  bool
  OnInit ();
};

IMPLEMENT_APP (yaDedaAppClass)

bool
yaDedaAppClass::OnInit ()
{
  int i;
  wxInitAllImageHandlers ();
  MainWindow = new MainFrame (NULL, wxID_ANY, wxEmptyString);

  cout
      << std::string (
	  "yaDEDA2 Apollo DEDA simulation, ver " VER(NVER) ", built " __DATE__ " " __TIME__ "\n");
  cout << std::string ("Copyright 2009 by Ronald S. Burkey\n");
  cout
      << std::string (
	  "Refer to http://www.ibiblio.org/apollo/index.html for more information.\n");

  Portnum = 19897;
  for (i = 1; i < argc; i++)
    {
      wxString Arg = argv[i];
      wxString ArgStart = Arg.BeforeFirst ('=');
      wxString ArgEnd = Arg.AfterFirst ('=');

      wxString msg;
      msg << "Arg " << i << " = \"" << Arg << "\"\n";
      cout << msg;
      if (Arg.IsSameAs (wxT ("--relative-pixmaps")))
	{
	  // Does nothing.  Legacy from yaDSKY.
	}
      else if (ArgStart.IsSameAs (wxT ("--ip")))
	{
	  strcpy (NonDefaultHostname, ArgEnd.char_str ());
	  Hostname = NonDefaultHostname;
	}
      else if (ArgStart.IsSameAs (wxT ("--port")))
	{
	  long lPortnum;
	  ArgEnd.ToLong (&lPortnum);
	  Portnum = lPortnum;
	  if (Portnum <= 0 || Portnum >= 0x10000)
	    {
	      printf ("The --port switch is out of range.  Must be 1-64K.\n");
	      goto Help;
	    }
	}
      else if (Arg.IsSameAs (wxT ("--half-size")))
	{
	  MainWindow->HalfSize = true;
	  MainWindow->HalveTheWindow ();
	}
      else if (ArgStart.IsSameAs (wxT ("--delay")))
	{
	  long lj;
	  ArgEnd.ToLong (&lj);
	  StartupDelay = lj;
	}
      else if (Arg.IsSameAs (wxT ("--show-packets")))
	{
	  // Does nothing.  Legacy from yaDEDA.
	}
      else
	{
	  Help: printf ("USAGE:\n");
	  printf ("\tyaDEDA2 [OPTIONS]\n");
	  printf ("The available options are:\n");
	  printf ("--ip=Hostname\n");
	  printf (
	      "\tThe yaDEDA2 program and the yaAGS Abort Guidance Computer simulation\n");
	  printf (
	      "\texist in a \"client/server\" relationship, in which the yaDEDA2 program\n");
	  printf (
	      "\tneeds to be aware of the IP address or symbolic name of the host \n");
	  printf (
	      "\tcomputer running the yaAGS program.  By default, this is \"localhost\",\n");
	  printf (
	      "\tmeaning that both yaDEDA2 and yaAGS are running on the same computer.\n");
	  printf ("--port=Portnumber\n");
	  printf (
	      "\tBy default, yaDEDA2 attempts to connect to the yaAGS program using port\n");
	  printf (
	      "\tnumber %d.  However, if more than one instance of yaDEDA2 is being\n",
	      Portnum);
	  printf (
	      "\trun, or if yaAGS has been configured to listen on different ports, then\n");
	  printf (
	      "\tdifferent port settings for yaDEDA2 are needed.  Note that by default,\n");
	  printf ("\tyaAGS listens for new connections on ports %d-%d.\n",
		  Portnum, Portnum + 10 - 1);
	  printf ("--half-size\n");
	  printf (
	      "\tUses a half-size version of yaDEDA2, suitable for smaller graphical\n");
	  printf ("\tdisplays. \n");
	  printf ("--delay=N\n");
	  printf (
	      "\t\"Start-up delay\", in ms.  Defaults to %d.  What the start-up\n",
	      StartupDelay);
	  printf (
	      "\tdelay does is to prevent yaDEDA2 from attempting to communicate with\n");
	  printf (
	      "\tyaAGS for a brief time after power-up.  This option is really only\n");
	  printf (
	      "\tuseful in Win32, to work around a problem with race-conditions at\n");
	  printf ("\tstart-up.\n");
	  printf ("--show-packets\n");
	  printf (
	      "\tDoes nothing.  The switch exists only for backward compatibility.\n");
	  printf ("--relative-pixmaps\n");
	  printf (
	      "\tDoes nothing. This switch is accepted only for backward compatibility.\n");
	  exit (1);
	}
    }
  wxString msg;
  msg << "Hostname=" << Hostname << ", port=" << Portnum << "\n";
  cout << msg;

  SetTopWindow (MainWindow);

  MainWindow->Timer = new TimerClass ();
  MainWindow->Timer->Start (PULSE_INTERVAL);

  MainWindow->Show ();
  return true;
}

//-------------------------------------------------------------------------
// This function is called every PULSE_INTERVAL milliseconds.  It manages
// the server connection, and causes display-updates based on input from
// yaAGS.

void
TimerClass::Notify ()
{

#if 0
  // Just a preliminary debugging thing, to check out how well bitmap
  // replacement works. It simply increments the count on the 5-digit
  // readout once/second.
  static int i, k = 0, Digits[5] =
    { 0};
  k++;
  if (k >= 10)
    {
      k = 0;
      // Update the in-memory counter.
      for (i = 0; i < 5; i++)
	{
	  Digits[i]++;
	  if (Digits[i] < 10)
	  break;
	  Digits[i] = 0;
	}
      MainWindow->ImageSet (MainWindow->DigitBottom5, DigitFilenames[Digits[0]]);
      MainWindow->ImageSet (MainWindow->DigitBottom4, DigitFilenames[Digits[1]]);
      MainWindow->ImageSet (MainWindow->DigitBottom3, DigitFilenames[Digits[2]]);
      MainWindow->ImageSet (MainWindow->DigitBottom2, DigitFilenames[Digits[3]]);
      MainWindow->ImageSet (MainWindow->DigitBottom1, DigitFilenames[Digits[4]]);
    }
#endif

  static unsigned char Packet[4];
  static int PacketSize = 0;
  int i;
  unsigned char c;
  if (StartupDelay > 0)
    {
      StartupDelay -= PULSE_INTERVAL;
      return;
    }
  // Try to connect to the server (yaAGC) if not already connected.
  if (ServerSocket == -1)
    {
      ServerSocket = CallSocket (Hostname, Portnum);
      if (ServerSocket != -1)
	printf ("yaDEDA2 is connected, socket=%d.\n", ServerSocket);
    }
  if (ServerSocket != -1)
    {
      for (;;)
	{
	  i = recv (ServerSocket, (char *) &c, 1, MSG_NOSIGNAL);
	  if (i == -1)
	    {
	      // The conditions i==-1,errno==0 or 9 occur only on Win32,
	      // and I'm not sure exactly what they corresponds to---but
	      // empirically I find that ignoring them makes no difference
	      // to the operation of the program.
	      if (errno == EAGAIN || errno == 0 || errno == 9)
		i = 0;
	      else
		{
		  printf ("yaDEDA2 reports server error %d\n", errno);
		  close (ServerSocket);
		  ServerSocket = -1;
		  break;
		}
	    }
	  if (i == 0)
	    break;
	  if (0 == (0xc0 & c))
	    PacketSize = 0;
	  if (PacketSize != 0 || (0xc0 & c) == 0)
	    {
	      Packet[PacketSize++] = c;
	      if (PacketSize >= 4)
		{
		  MainWindow->ActOnIncomingIO (Packet);
		  PacketSize = 0;
		}
	    }
	}
    }
}

// A function used to process a collected incoming packet from the AGS.
void
MainFrame::ActOnIncomingIO (unsigned char *Packet)
{
  extern int ShowPackets;
  int Type, Data;
  clock_t TimeRightNow;
  struct tms TmsBuf;

  // Check to see if the message has a yaAGS signature.  If not,
  // ignore it.  The yaAGS signature is 00 11 10 01 in the 
  // 2 most-significant bits of the packet's bytes.  We are 
  // guaranteed that the first byte is signed 00, so we don't 
  // need to check it.
  if (0xc0 != (Packet[1] & 0xc0) || 0x80 != (Packet[2] & 0xc0)
      || 0x40 != (Packet[3] & 0xc0))
    return;
  // Make sure we know how to find all of the widgets we need.
  //LocateWidgets (widget);
  // What's in this packet?
  if (ParseIoPacketAGS (Packet, &Type, &Data))
    {
      return;			// Oops!  Error in packet.
    }
  // Only a few types are of interest to us.  Note that the DEDA Shift Out discrete
  // isn't of interest to us, since it's merely internal to yaAGS, and the data
  // will simply appear in the DEDA shift register.  In other words, we never
  // even get to see it as anything other than 0.
  if (Type == 040 && 0 == (Data & 010))	// DEDA Shift In discrete, active 0
    {
      int Data;
      // Account for timeout in data to reset the packet buffer.
      TimeRightNow = times (&TmsBuf);
      if (ShiftingOut && ShiftBufferPtr == 0)
	WhenOutPacket = TimeRightNow;
      if (TimeRightNow >= WhenOutPacket + WHEN_OUT_EXPIRED)
	ShiftingOut = 0;
      WhenOutPacket = TimeRightNow + WHEN_OUT_EXPIRED;
      // Prepare the data.
      if (!ShiftingOut || ShiftBufferPtr >= ShiftBufferSize)
	{
	  Data = 0x0f << 13;
	  ShiftingOut = 0;
	}
      else
	{
	  Data = ShiftBuffer[ShiftBufferPtr++] << 13;
	  if (ShiftBufferPtr >= ShiftBufferSize)
	    ShiftingOut = 0;
	}
      // Send the data.
      OutputData (07, Data);
      // Key releases
      if (ShiftingOut == 0)
	{
	  wxMilliSleep (100);
	  if (LastReadout)
	    OutputData (05, 0777002);	// Release READ OUT key.
	  else
	    OutputData (05, 0777004);	// Release ENTR key.
	}
    }
  else if (Type == 027)				// Incoming DEDA shift register.
    {
      // Account for timeout in data to reset the packet buffer.
      TimeRightNow = times (&TmsBuf);
      if (TimeRightNow >= WhenInPacket + WHEN_IN_EXPIRED)
	NumInPacket = 0;
      WhenInPacket = TimeRightNow + WHEN_IN_EXPIRED;
      // Save the data.  Only the values 0-9 and 15 are valid, but we don't
      // bother to check.
      InPacket[NumInPacket++] = (Data >> 13) & 0x0F;
      if (NumInPacket >= 9)
	{
	  // Okay, we've received a complete packet from the AEA.  Let's 
	  // overwrite the display.
	  NumInPacket = 0;
	  ImageSet (DigitTop1, DigitFilenames[InPacket[0]]);
	  ImageSet (DigitTop2, DigitFilenames[InPacket[1]]);
	  ImageSet (DigitTop3, DigitFilenames[InPacket[2]]);
	  ImageSet (Sign, SignFilenames[InPacket[3]]);
	  ImageSet (DigitBottom1, DigitFilenames[InPacket[4]]);
	  ImageSet (DigitBottom2, DigitFilenames[InPacket[5]]);
	  ImageSet (DigitBottom3, DigitFilenames[InPacket[6]]);
	  ImageSet (DigitBottom4, DigitFilenames[InPacket[7]]);
	  ImageSet (DigitBottom5, DigitFilenames[InPacket[8]]);
	}
    }
}

//--------------------------------------------------------------------------------
// A nice little function to output a packet to yaAGS.

void
MainFrame::OutputData (int Type, int Data)
{
  unsigned char Packet[4];
  extern int ServerSocket;
  int j;
  if (ServerSocket != -1)
    {
      FormIoPacketAGS (Type, Data, Packet);
      j = send (ServerSocket, (const char *) Packet, 4, MSG_NOSIGNAL);
      if (j == SOCKET_ERROR && SOCKET_BROKEN)
	{
	  close (ServerSocket);
	  ServerSocket = -1;
	}
    }
}

//--------------------------------------------------------------------------------
// When this function is called, the line buffer contains either 3 or 9 characters
// for shifting out to the AEA.  Data is shifted out from ShiftBuffer, allowing
// OutPacket to be used for buffering keystrokes again.

void
MainFrame::ReadyForShift (void)
{
  ShiftingOut = 1;
  for (ShiftBufferSize = 0; ShiftBufferSize < NumOutPacket; ShiftBufferSize++)
    ShiftBuffer[ShiftBufferSize] = OutPacket[ShiftBufferSize];
  ShiftBufferPtr = 0;
  NumOutPacket = 10;
}

//--------------------------------------------------------------------------------
// Clear the display.

void
MainFrame::ClearTheDisplay (void)
{
  ImageSet (DigitTop1, DigitFilenames[15]);
  ImageSet (DigitTop2, DigitFilenames[15]);
  ImageSet (DigitTop3, DigitFilenames[15]);
  ImageSet (Sign, SignFilenames[15]);
  ImageSet (DigitBottom1, DigitFilenames[15]);
  ImageSet (DigitBottom2, DigitFilenames[15]);
  ImageSet (DigitBottom3, DigitFilenames[15]);
  ImageSet (DigitBottom4, DigitFilenames[15]);
  ImageSet (DigitBottom5, DigitFilenames[15]);
}

//--------------------------------------------------------------------------------
// Process a digit as a result of a keypad entry.

void
MainFrame::ProcessDigit (int Condition, int Num)
{
  LastReadout = 0;
  OperatorError (Condition);
  if (OprErr)
    return;
  switch (NumOutPacket)
    {
    case 0:
      ImageSet (DigitTop1, DigitFilenames[Num]);
      break;
    case 1:
      ImageSet (DigitTop2, DigitFilenames[Num]);
      break;
    case 2:
      ImageSet (DigitTop3, DigitFilenames[Num]);
      break;
    case 3:
      ImageSet (Sign, SignFilenames[Num]);
      break;
    case 4:
      ImageSet (DigitBottom1, DigitFilenames[Num]);
      break;
    case 5:
      ImageSet (DigitBottom2, DigitFilenames[Num]);
      break;
    case 6:
      ImageSet (DigitBottom3, DigitFilenames[Num]);
      break;
    case 7:
      ImageSet (DigitBottom4, DigitFilenames[Num]);
      break;
    case 8:
      ImageSet (DigitBottom5, DigitFilenames[Num]);
      break;
    default:
      OperatorError (1);
      break;
    }
  if (NumOutPacket < 9)
    OutPacket[NumOutPacket++] = Num;
}

//--------------------------------------------------------------------------------
// Activate or deactivate the OPR ERR indicator.

void
MainFrame::OperatorError (int On)
{
  On = (On != 0);			// Make sure it's 0 or 1.
  if (On != OprErr)
    {
      OprErr = On;
      ImageSet (IndicatorOprErr, OprErrFilenames[On]);
      if (On)
	{
	  NumOutPacket = 10;
	  ClearTheDisplay ();
	}
    }
}

// A function to overwrite one of the static bitmaps in the display, such as an 
// annunciator, with a replacement from a file.

void
MainFrame::ImageSet (wxStaticBitmap *StaticBitmap, wxString &Filename)
{
  wxString Dummy;
  wxBitmap Bitmap;
  if (HalfSize)
    Dummy = wxT ("h") + Filename;
  else
    Dummy = Filename;
  Bitmap = StaticBitmap->GetBitmap ();
  Bitmap.LoadFile (Dummy, wxBITMAP_TYPE_XPM);
  StaticBitmap->SetBitmap (Bitmap);
}
void
MainFrame::ImageSet (wxStaticBitmap *StaticBitmap, char *Filename)
{
  wxString Dummy;
  Dummy = wxString::FromAscii (Filename);
  ImageSet (StaticBitmap, Dummy);
}
void
MainFrame::ImageSet (wxStaticBitmap *StaticBitmap, const char *Filename)
{
  ImageSet (StaticBitmap, (char *) Filename);
}
void
MainFrame::ImageSet (wxBitmapButton *BitmapButton, wxString &Filename)
{
  wxString Dummy;
  wxBitmap Bitmap;
  if (HalfSize)
    Dummy = wxT ("h") + Filename;
  else
    Dummy = Filename;
  Bitmap = BitmapButton->GetBitmapLabel ();
  Bitmap.LoadFile (Dummy, wxBITMAP_TYPE_XPM);
  BitmapButton->SetBitmapLabel (Bitmap);
}
void
MainFrame::ImageSet (wxBitmapButton *BitmapButton, char *Filename)
{
  wxString Dummy;
  Dummy = wxString::FromAscii (Filename);
  ImageSet (BitmapButton, Dummy);
}
void
MainFrame::ImageSet (wxBitmapButton *BitmapButton, const char *Filename)
{
  ImageSet (BitmapButton, (char *) Filename);
}

// The following function was adapted from the normal constructor.  It is called only at the point in the
// startup sequence where it has noted that the --half-size switch has been used, and it is called to
// replace the already-created but not-yet-displayed main window to use smaller graphics.
void
MainFrame::HalveTheWindow (void)
{
  int ButtonSize, Thickness;
  if (HalfSize)
    {
      ButtonSize = 40;
      Thickness = 2;
    }
  else
    {
      ButtonSize = 75;
      Thickness = 4;
    }
  ImageSet (bitmap_10, "BorderHorizontal158.xpm");
  ImageSet (bitmap_5, "BorderVertical48.xpm");
  ImageSet (DigitTop1, "7SegOff.xpm");
  ImageSet (DigitTop2, "7SegOff.xpm");
  ImageSet (DigitTop3, "7SegOff.xpm");
  ImageSet (bitmap_5_copy, "BorderVertical48.xpm");
  ImageSet (bitmap_11, "BorderHorizontal158.xpm");
  ImageSet (bitmap_12, "BorderHorizontal88.xpm");
  ImageSet (bitmap_5_copy_1, "BorderVertical48.xpm");
  ImageSet (IndicatorOprErr, "OprErrOff.xpm");
  ImageSet (bitmap_20, "BorderVertical48.xpm");
  ImageSet (bitmap_12_copy, "BorderHorizontal88.xpm");
  ImageSet (bitmap_13, "BorderHorizontal290.xpm");
  ImageSet (bitmap_7, "BorderVertical48.xpm");
  ImageSet (Sign, "PlusMinusOff.xpm");
  ImageSet (DigitBottom1, "7SegOff.xpm");
  ImageSet (DigitBottom2, "7SegOff.xpm");
  ImageSet (DigitBottom3, "7SegOff.xpm");
  ImageSet (DigitBottom4, "7SegOff.xpm");
  ImageSet (DigitBottom5, "7SegOff.xpm");
  ImageSet (bitmap_7_copy, "BorderVertical48.xpm");
  ImageSet (bitmap_14, "BorderHorizontal290.xpm");
  ImageSet (KeyPlus, "PlusUp.xpm");
  ImageSet (Key7, "7Up.xpm");
  ImageSet (Key8, "8Up.xpm");
  ImageSet (Key9, "9Up.xpm");
  ImageSet (KeyClr, "ClrUp.xpm");
  ImageSet (KeyMinus, "MinusUp.xpm");
  ImageSet (Key4, "4Up.xpm");
  ImageSet (Key5, "5Up.xpm");
  ImageSet (Key6, "6Up.xpm");
  ImageSet (KeyReadOut, "ReadOutUp.xpm");
  ImageSet (Key0, "0Up.xpm");
  ImageSet (Key1, "1Up.xpm");
  ImageSet (Key2, "2Up.xpm");
  ImageSet (Key3, "3Up.xpm");
  ImageSet (KeyEntr, "EntrUp.xpm");
  ImageSet (KeyHold, "HoldUp.xpm");
  KeyPlus->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key7->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key8->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key9->SetMinSize (wxSize (ButtonSize, ButtonSize));
  KeyClr->SetMinSize (wxSize (ButtonSize, ButtonSize));
  KeyMinus->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key4->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key5->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key6->SetMinSize (wxSize (ButtonSize, ButtonSize));
  KeyReadOut->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key0->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key1->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key2->SetMinSize (wxSize (ButtonSize, ButtonSize));
  Key3->SetMinSize (wxSize (ButtonSize, ButtonSize));
  KeyEntr->SetMinSize (wxSize (ButtonSize, ButtonSize));
  KeyHold->SetMinSize (wxSize (ButtonSize, ButtonSize));
  bitmap_5->SetMinSize (wxSize (Thickness, 23));
  bitmap_5_copy->SetMinSize (wxSize (Thickness, 23));
  bitmap_12->SetMinSize (wxSize (48, Thickness));
  bitmap_5_copy_1->SetMinSize (wxSize (Thickness, 21));
  bitmap_20->SetMinSize (wxSize (Thickness, 21));
  bitmap_12_copy->SetMinSize (wxSize (48, Thickness));
  bitmap_7->SetMinSize (wxSize (Thickness, 22));
  bitmap_7_copy->SetMinSize (wxSize (Thickness, 22));
  Fit ();
}

