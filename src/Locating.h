
#ifndef LOCATING_CYCLE_CLASS_DEFINITION
#define LOCATING_CYCLE_CLASS_DEFINITION

// Locating.h
/*
 * Copyright (c) 2009, Dan Heeks, Perttu Ahola
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#include "Op.h"
#include "HeeksCNCTypes.h"
#include <list>
#include <vector>
#include "CNCPoint.h"

class CLocating;

class CLocatingParams{

public:
	double m_standoff;		// This is the height above the staring Z position that forms the Z retract height (R word)
	int    m_sort_locations;	// Perform a location-based sort before generating GCode?

	// The following line is the prototype setup in the Python routines for the drill sequence.
	// def drill(x=None, y=None, z=None, depth=None, standoff=None, dwell=None, peck_depth=None):

	void set_initial_values();
	void write_values_to_config();
	void GetProperties(CLocating* parent, std::list<Property *> *list);
	void WriteXMLAttributes(TiXmlNode* pElem);
	void ReadParametersFromXMLElement(TiXmlElement* pElem);

	wxString ConfigScope() const { return(_("Locating")); }
};

/**
	The CLocating class stores a list of symbols (by type/id pairs) of elements that represent some point
	at which the user wants the to place the machine and then pause.  One scenario being where a previous
	drilling operation drilled a series of holes and then a Locating operation moves the machine to
	a position immediately above these holes and then pauses operations.  This allows a manual tapping
	head to be used.  When the hole has been tapped, the user can then resume operations at which point
	the machine will be located above the next hole ready for another manual operation.

	It also accepts references to circle objects.  For this special case, the circle may not intersect any of
	the other objects.  If this is the case then the circle's centre will be used as a hole location.

	Finally the code tries to intersect all selected objects and places holes (Locating Cycles) at all
	intersection points.
 */

class CLocating: public COp {
public:
	/**
		Define some data structures to hold references to CAD elements.  We store both the type and id because
			a) the ID values are only relevant within the context of a type.
			b) we don't want to limit this class to PointType elements alone.  We use these
			   symbols to identify pairs of intersecting elements and place a drilling cycle
			   at their intersection.
 	 */
	typedef int SymbolType_t;
	typedef unsigned int SymbolId_t;
	typedef std::pair< SymbolType_t, SymbolId_t > Symbol_t;
	typedef std::list< Symbol_t > Symbols_t;

public:
	//	These are references to the CAD elements whose position indicate where the Locating Operation occurs.
	//	If the m_params.m_sort_locations is false then the order of symbols in this list should
	//	be respected when generating GCode.  We will, eventually, allow a user to sort the sub-elements
	//	visually from within the main user interface.  When this occurs, the change in order should be
	//	reflected in the ordering of symbols in the m_symbols list.

	Symbols_t m_symbols;
	CLocatingParams m_params;

	//	Constructors.
	CLocating():COp(GetTypeString(), 0, LocatingType){}
	CLocating(	const Symbols_t &symbols )
		: COp(GetTypeString(), 0, LocatingType), m_symbols(symbols)
	{
		m_params.set_initial_values();

		for (Symbols_t::const_iterator symbol = symbols.begin(); symbol != symbols.end(); symbol++)
		{
			HeeksObj *object = heeksCAD->GetIDObject( symbol->first, symbol->second );
			if (object != NULL)
			{
				Add( object, NULL );
			} // End if - then
		} // End for

		m_symbols.clear();	// we don't want to do this twice.
	}

	CLocating( const CLocating & rhs );
	CLocating & operator= ( const CLocating & rhs );

	// HeeksObj's virtual functions
	int GetType()const{return LocatingType;}
	const wxChar* GetTypeString(void)const{return _T("Locating");}
	void glCommands(bool select, bool marked, bool no_color);

	void GetIcon(int& texture_number, int& x, int& y){if(m_active){GET_ICON(6, 0);}else COp::GetIcon(texture_number, x, y);}
	void GetProperties(std::list<Property *> *list);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object);
	void WriteXML(TiXmlNode *root);
	bool CanAddTo(HeeksObj* owner);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void ReloadPointers();

	static bool ValidType( const int object_type );

	// This is the method that gets called when the operator hits the 'Python' button.  It generates a Python
	// program whose job is to generate RS-274 GCode.
	void AppendTextToProgram( const CFixture *pFixture );

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	void AddSymbol( const SymbolType_t type, const SymbolId_t id ) { m_symbols.push_back( Symbol_t( type, id ) ); }
	std::vector<CNCPoint> FindAllLocations();

	std::list<wxString> DesignRulesAdjustment(const bool apply_changes);
};


#endif // LOCATING_CYCLE_CLASS_DEFINITION
