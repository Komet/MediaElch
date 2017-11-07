/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "ZenLib/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(ZENLIB_DEBUG)
//---------------------------------------------------------------------------
#include <iomanip>
#include <sstream>
#include "ZenLib/MemoryDebug.h"
#include "ZenLib/Ztring.h"
#ifdef WINDOWS
    #include <io.h>
#else
    #include <cstdio>
#endif
#include <fcntl.h>
#include <sys/stat.h>
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

bool MemoryDebug::g_IsShutdown = false;
//***************************************************************************
// Constructors/destructor
//***************************************************************************

MemoryDebug::MemoryDebug()
{
}

MemoryDebug::~MemoryDebug()
{
    if (!m_Blocks.empty())
        ReportLeaks();
    g_IsShutdown = true;
}

//***************************************************************************
// Instance
//***************************************************************************

MemoryDebug& MemoryDebug::Instance()
{
    static MemoryDebug Inst;
    return Inst;
}

//***************************************************************************
// Reports
//***************************************************************************

void MemoryDebug::ReportLeaks()
{
    Ztring m_File;
    //std::ofstream      m_File ("Debug_MemoryLeak.txt");        // Fichier de sortie

    // D�tail des fuites
    std::size_t TotalSize = 0;
    for (TBlockMap::iterator i = m_Blocks.begin(); i != m_Blocks.end(); ++i)
    {
        // Ajout de la taille du bloc au cumul
        TotalSize += i->second.Size;

        // Inscription dans le fichier des informations sur le bloc courant
        /*
        m_File << "-> 0x" << std::hex << i->first << std::dec
               << " | "   << std::setw(7) << std::setfill(' ') << static_cast<int>(i->second.Size) << " bytes"
               << " | "   << i->second.File.c_str() << " (" << i->second.Line << ")" << std::endl;
        */
        m_File.append(__T("-> 0x"));
        m_File.append(Ztring::ToZtring((size_t)i->first, 16));
        m_File.append(__T(" | "));
        Ztring Temp;
        Temp.From_Number(static_cast<int>(i->second.Size));
        while(Temp.size()<7)
            Temp=__T(" ")+Temp;
        m_File.append(Temp);
        m_File.append(__T(" bytes"));
        m_File.append(__T(" | "));
        m_File.append(Ztring().From_Local(i->second.File.c_str()));
        m_File.append(__T(" ("));
        m_File.append(Ztring::ToZtring(i->second.Line));
        m_File.append(__T(")"));
        m_File.append(EOL);
    }

    // Affichage du cumul des fuites
    /*
    m_File << std::endl << std::endl << "-- "
           << static_cast<int>(m_Blocks.size()) << " non-released blocs, "
           << static_cast<int>(TotalSize)       << " bytes --"
           << std::endl;
    */
    m_File.append(EOL);
    m_File.append(EOL);
    m_File.append(__T("-- "));
    m_File.append(Ztring::ToZtring(static_cast<int>(m_Blocks.size())));
    m_File.append(__T(" non-released blocs, "));
    m_File.append(Ztring::ToZtring(static_cast<int>(TotalSize)));
    m_File.append(__T(" bytes --"));
    m_File.append(EOL);

    std::string ToWrite=m_File.To_Local().c_str();
    int m_File_sav=open("Debug_MemoryLeak.txt", O_BINARY|O_RDWR  |O_CREAT);        // Fichier de sortie
    write(m_File_sav, (int8u*)ToWrite.c_str(), ToWrite.size());
    close(m_File_sav);
}

//***************************************************************************
// Memory management
//***************************************************************************

void* MemoryDebug::Allocate(std::size_t Size, const char* File, int Line, bool Array)
{
    // Allocation de la m�moire
    void* Ptr = malloc(Size);

    // Ajout du bloc � la liste des blocs allou�s
    TBlock NewBlock;
    NewBlock.Size  = Size;
    NewBlock.File  = File;
    NewBlock.Line  = Line;
    NewBlock.Array = Array;
    m_Blocks[Ptr]  = NewBlock;
    return Ptr;
}

void MemoryDebug::Free(void* Ptr, bool Array)
{
    // Recherche de l'adresse dans les blocs allou�s
    TBlockMap::iterator It = m_Blocks.find(Ptr);

    // Si le bloc n'a pas �t� allou�, on g�n�re une erreur
    if (It == m_Blocks.end())
    {
        // En fait �a arrive souvent, du fait que le delete surcharge est pris en compte meme la ou on n'inclue pas DebugNew.h,
        // mais pas la macro pour le new
        // Dans ce cas on d�truit le bloc et on quitte imm�diatement
        free(Ptr);
        return;
    }

    // Si le type d'allocation ne correspond pas, on g�n�re une erreur
    if (It->second.Array != Array)
    {
        //throw CBadDelete(Ptr, It->second.File.c_str(), It->second.Line, !Array);
    }

    // Finalement, si tout va bien, on supprime le bloc et on loggiz tout �a
    m_Blocks.erase(It);
    m_DeleteStack.pop();

    // Lib�ration de la m�moire
    free(Ptr);
}

void MemoryDebug::NextDelete(const char* File, int Line)
{
    TBlock Delete;
    Delete.File = File;
    Delete.Line = Line;

    m_DeleteStack.push(Delete);
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace

#endif // defined(ZENLIB_DEBUG)
