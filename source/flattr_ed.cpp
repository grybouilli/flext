/* 

flext - C++ layer for Max/MSP and pd (pure data) externals

Copyright (c) 2001-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

/*! \file flattr_ed.cpp
    \brief Attribute editor (property dialog) for PD
*/

#include "flprefix.h"

#if FLEXT_SYS == FLEXT_SYS_PD && !defined(FLEXT_NOATTREDIT)

#ifdef _MSC_VER
#pragma warning( disable : 4091 ) 
#endif

#include <m_imp.h>
#include "flext.h"

#include <string.h>
#include <stdio.h>

#ifdef __MWERKS__
#define STD std
#else
#define STD
#endif


#if !defined(PD_VERSION_MAJOR)
	/* PD version 0.36 or below */

	/* Call this to get a gobj's bounding rectangle in pixels */
	typedef void (*t_getrectfn)(t_gobj *x, struct _glist *glist,
		int *x1, int *y1, int *x2, int *y2);
    		/* and this to displace a gobj: */
	typedef void (*t_displacefn)(t_gobj *x, struct _glist *glist, int dx, int dy);
    		/* change color to show selection: */
	typedef void (*t_selectfn)(t_gobj *x, struct _glist *glist, int state);
    		/* change appearance to show activation/deactivation: */
	typedef void (*t_activatefn)(t_gobj *x, struct _glist *glist, int state);
    		/* warn a gobj it's about to be deleted */
	typedef void (*t_deletefn)(t_gobj *x, struct _glist *glist);
    		/*  making visible or invisible */
	typedef void (*t_visfn)(t_gobj *x, struct _glist *glist, int flag);
    		/* field a mouse click (when not in "edit" mode) */
	typedef int (*t_clickfn)(t_gobj *x, struct _glist *glist,
		int xpix, int ypix, int shift, int alt, int dbl, int doit);
    		/*  save to a binbuf */
	typedef void (*t_savefn)(t_gobj *x, t_binbuf *b);
    		/*  open properties dialog */
	typedef void (*t_propertiesfn)(t_gobj *x, struct _glist *glist);
    		/* ... and later, resizing; getting/setting font or color... */

	struct _widgetbehavior
	{
		t_getrectfn w_getrectfn;
		t_displacefn w_displacefn;
		t_selectfn w_selectfn;
		t_activatefn w_activatefn;
		t_deletefn w_deletefn;
		t_visfn w_visfn;
		t_clickfn w_clickfn;
		t_savefn w_savefn;
		t_propertiesfn w_propertiesfn;
	};
	
#elif !defined(PD_VERSION_MINOR)
	#error Flext cannot be compiled with this version!
#else
	#include <g_canvas.h>
#endif


static t_widgetbehavior widgetbehavior; 
static void (*ori_vis)(t_gobj *c, t_glist *, int vis) = NULL;

void flext_base::SetAttrEditor(t_classid c)
{
	// widgetbehavior struct MUST be resident... (static is just ok here)

    widgetbehavior.w_getrectfn =    c->c_wb->w_getrectfn; 
    widgetbehavior.w_displacefn =   c->c_wb->w_displacefn; 
    widgetbehavior.w_selectfn =     c->c_wb->w_selectfn; 
    widgetbehavior.w_activatefn =   c->c_wb->w_activatefn; 
    widgetbehavior.w_deletefn =     c->c_wb->w_deletefn; 
	ori_vis = c->c_wb->w_visfn;
    widgetbehavior.w_visfn =        cb_GfxVis;
    widgetbehavior.w_clickfn =      c->c_wb->w_clickfn;
    widgetbehavior.w_propertiesfn = cb_GfxProperties;
    widgetbehavior.w_savefn =       cb_GfxSave;
    class_setwidget(c, &widgetbehavior);


	// generate the script for the property dialog

	sys_gui(
		"proc flext_apply {id alen} {\n"
			// strip "." from the TK id to make a variable name suffix
			"set vid [string trimleft $id .]\n"

			// make a list of the attribute values (including save flags)
			"set lst {}\n"
			"for {set ix 0} {$ix < $alen} {incr ix} {\n"
				"set var_attr_name [concat [concat var_name_$ix]_$vid ]\n"
				"global $var_attr_name\n"
				"set var_attr_val [concat [concat var_val_$ix]_$vid ]\n"
				"global $var_attr_val\n"
				"set var_attr_save [concat [concat var_save_$ix]_$vid ]\n"
				"global $var_attr_save\n"
		
				"lappend lst [eval concat $$var_attr_name]\n" 
				// see if it's a list
				"set len [llength [expr $$var_attr_val]]\n"
				"if { $len > 1 } {\n"
					"set lst [concat $lst \"list\" $len [expr $$var_attr_val]]\n" 
				"} else {\n"
					"lappend lst [expr $$var_attr_val]\n" 
				"}\n"
				"lappend lst [eval concat $$var_attr_save]\n" 
			"}\n"

			"set cmd [concat $id attributedialog $lst \\;]\n"
			// puts stderr $cmd
			"pd $cmd\n"
		"}\n"

		"proc flext_cancel {id} {\n"
			"set cmd [concat $id cancel \\;]\n"
			// puts stderr $cmd
			"pd $cmd\n"
		"}\n"

		"proc flext_ok {id alen} {\n"
			"flext_apply $id $alen\n"
			"flext_cancel $id\n"
		"}\n"

		"proc pdtk_flext_dialog {id attrlist} {\n"
				"set vid [string trimleft $id .]\n"
				"set alen [expr [llength $attrlist] / 3 ]\n"

				"toplevel $id\n"
				"wm title $id {object attributes}\n"
				"wm protocol $id WM_DELETE_WINDOW [concat flext_cancel $id]\n"
/*
				"label $id.label -text {Attributes}\n"
				"pack $id.label -side top\n"
*/
				"set ix 0\n"

				"foreach {an av asv} $attrlist {\n"
					"set nm [concat $id.nm-$ix]\n"

					"set var_attr_name [concat [concat var_name_$ix]_$vid ]\n"
					"global $var_attr_name\n"
					"set $var_attr_name $an\n"

					"set var_attr_val [concat [concat var_val_$ix]_$vid ]\n"
					"global $var_attr_val\n"
					"set $var_attr_val $av\n"

					"set var_attr_save [concat [concat var_save_$ix]_$vid ]\n"
					"global $var_attr_save\n"
					"set $var_attr_save $asv\n"

					"frame $nm\n"
					"pack $nm -side top\n"
					"label $nm.lwidth -text \"$an :\"\n"
					"entry $nm.width -textvariable $var_attr_val -width 20\n"
					"checkbutton $nm.save -text {save} -variable $var_attr_save -anchor w\n"
					"pack $nm.lwidth $nm.width $nm.save -side left\n"
					"bind $nm.width <KeyPress-Return> [concat flext_ok $id $alen]\n"

					"incr ix\n"
				"}\n"

//				"focus $id.1-rangef.width\n"


				// Buttons
				
				"frame $id.buttonframe\n"
				"pack $id.buttonframe -side bottom -fill x -pady 2m\n"

				"button $id.buttonframe.cancel -text {Cancel} -command \"flext_cancel $id\"\n"
				"button $id.buttonframe.apply -text {Apply} -command \"flext_apply $id $alen\"\n"
				"button $id.buttonframe.ok -text {OK} -command \"flext_ok $id $alen\"\n"

				"pack $id.buttonframe.cancel -side left -expand 1\n"
				"pack $id.buttonframe.apply -side left -expand 1\n"
				"pack $id.buttonframe.ok -side left -expand 1\n"

		"}\n"
	);
}


void flext_base::cb_GfxProperties(t_gobj *c, t_glist *)
{
	flext_base *th = thisObject(c);
	char buf[1000],*b = buf;

	AtomList la;
	int cnt = th->ListAttrib(la);

	STD::sprintf(b, "pdtk_flext_dialog %%s { "); b += strlen(b);

	for(int i = 0; i < cnt; ++i) {
		STD::sprintf(b,"%s {",GetString(la[i])); b += strlen(b);

		bool sv;

		// get attribute
		attritem *attr = th->FindAttrib(GetSymbol(la[i]),true);

		if(attr) {
			// Attribute is gettable

			// Get attribute value
			AtomList lv;
			th->GetAttrib(attr,lv);

			sv = th->GetAttribSave(attr);

			for(int j = 0; j < lv.Count(); ++j) {
				if(IsString(lv[j]))
					STD::sprintf(b,"%s",GetString(lv[j]));
				else if(IsFloat(lv[j]))
					STD::sprintf(b,"%f",GetFloat(lv[j]));
				else if(IsInt(lv[j]))
					STD::sprintf(b,"%i",GetInt(lv[j]));
				else
					FLEXT_ASSERT(false);
				b += strlen(b);

				if(j < lv.Count()-1) *(b++) = ' ';
			}
		}
		else {
			// Attribute is not gettable
			sv = false;

			// \TODO set flag for tcl/tk dialog
		}

		STD::sprintf(b, "} %i ", sv?1:0); b += strlen(b);
	}

	strcpy(b, " }\n");

	gfxstub_new((t_pd *)th->thisHdr(), th->thisHdr(), buf);
}

//! Strip the attributes off the object command line
void flext_base::cb_GfxVis(t_gobj *c, t_glist *gl, int vis)
{
	flext_base *th = thisObject(c);
	t_text *x = (t_text *)th->thisHdr();
	FLEXT_ASSERT(x->te_binbuf);

	int argc = binbuf_getnatom(x->te_binbuf);
	t_atom *argv = binbuf_getvec(x->te_binbuf);
	int cnt = CheckAttrib(argc,argv);

	if(cnt)	{
		t_binbuf *nb = binbuf_new();
		binbuf_restore(nb,cnt,argv);
		binbuf_free(x->te_binbuf);
		x->te_binbuf = nb;
	}

	ori_vis(c,gl,vis);
}

void flext_base::cb_GfxSave(t_gobj *c, t_binbuf *b)
{
	flext_base *th = thisObject(c);
	t_text *t = (t_text *)c;
	binbuf_addv(b, "ssiis", gensym("#X"),gensym("obj"),	t->te_xpix, t->te_ypix,MakeSymbol(th->thisName()));

	int argc = binbuf_getnatom(t->te_binbuf);
	t_atom *argv = binbuf_getvec(t->te_binbuf);
	int cnt = CheckAttrib(argc,argv);

	// process the creation arguments
	for(int i = 1; i < cnt; ++i) {
		if(IsString(argv[i]))
			binbuf_addv(b,"s",GetSymbol(argv[i]));
		else if(IsFloat(argv[i]))
			binbuf_addv(b,"f",GetFloat(argv[i]));
		else if(IsInt(argv[i]))
			binbuf_addv(b,"i",GetInt(argv[i]));
		else
			FLEXT_ASSERT(false);
	}

	// process the attributes
	AtomList la;
	cnt = th->ListAttrib(la);
	char attrname[100];
	*attrname= '@';

	for(int i = 0; i < cnt; ++i) {
		// must be both settable and gettable....
		attritem *attr = th->FindAttrib(GetSymbol(la[i]),true);

		if(attr && attr->BothExist() && th->GetAttribSave(attr)) {
			// Get attribute value
			AtomList lv;
			th->GetAttrib(attr,lv);

			strcpy(attrname+1,GetString(la[i]));
			binbuf_addv(b,"s",MakeSymbol(attrname));

			for(int j = 0; j < lv.Count(); ++j) {
				if(IsString(lv[j]))
					binbuf_addv(b,"s",GetSymbol(lv[j]));
				else if(IsFloat(lv[j]))
					binbuf_addv(b,"f",GetFloat(lv[j]));
				else if(IsInt(lv[j]))
					binbuf_addv(b,"i",GetInt(lv[j]));
				else
					FLEXT_ASSERT(false);
			}
		}
	}

	binbuf_addv(b, ";");
}

bool flext_base::cb_AttrDialog(flext_base *th,int argc,const t_atom *argv)
{
	int i = 0;
	if(IsSymbol(argv[i]) && GetSymbol(argv[i]) == sym_list) ++i;

	for(; i < argc; ) {
		FLEXT_ASSERT(IsSymbol(argv[i]));

		const t_symbol *aname = GetSymbol(argv[i]);
		i++;

		int cnt,offs;
		if(IsSymbol(argv[i]) && GetSymbol(argv[i]) == sym_list) {
			i++;
			FLEXT_ASSERT(CanbeInt(argv[i]));
			cnt = GetAInt(argv[i]);
			offs = ++i;
		}
		else
			offs = i,cnt = 1;

		i += cnt;

		FLEXT_ASSERT(i < argc);
		bool sv = CanbeBool(argv[i]) && GetABool(argv[i]);
		++i;

		// find settable attribute
		attritem *attr = th->FindAttrib(aname,false);
		if(attr) {
			th->SetAttribSave(attr,sv);
			bool ret = th->SetAttrib(attr,cnt,argv+offs);
			FLEXT_ASSERT(ret);
		}
		else {
			post("%s - Attribute %s can't be set",th->thisName(),GetString(aname));
		}
	}
	return true;
}


#endif // FLEXT_SYS_PD