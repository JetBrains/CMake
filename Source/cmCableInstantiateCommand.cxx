/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmCabilInstantiateCommand.h"
#include "cmCacheManager.h"

#include "cmCabilDefineSetCommand.h"

// cmCabilInstantiateCommand
bool cmCabilInstantiateCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // This command instance needs to use the cmCabilData instance.
  this->SetupCabilData();

  std::vector<std::string>::const_iterator arg = args.begin();
  
  // Get the output file into which the configuration code is to be
  // written.  The name of the file is the first argument.
  m_OutputFile = m_CabilData->GetOutputFile(*arg++, this);
  
  // The rest of the arguments are the elements to be placed in the set.
  for(; arg != args.end(); ++arg)
    {
    m_Elements.push_back(*arg);
    }  
  
  return true;
}


void cmCabilInstantiateCommand::FinalPass()
{
  // If this command is the first to reference its output file, write the
  // header information.
  if(m_OutputFile->FirstReferencingCommandIs(this))
    {
    this->WriteConfigurationHeader(m_OutputFile->GetStream());
    
    // Need to write out the Set definitions.
    // Look through the vector of commands from the makefile.
    const std::vector<cmCommand*>& usedCommands =
      m_Makefile->GetUsedCommands();  
    for(std::vector<cmCommand*>::const_iterator commandIter =
          usedCommands.begin();
        commandIter != usedCommands.end(); ++commandIter)
      {
      // If this command is a cmCabilDefineSetCommand, ask it to write its
      // configuration code to the output file.
      cmCabilDefineSetCommand* command =
        cmCabilDefineSetCommand::SafeDownCast(*commandIter);
      if(command)
        {
        command->WriteConfiguration(m_OutputFile->GetStream());
        }
      }
    }  
  
  // Write the instantiation block's code.
  this->WriteConfiguration(m_OutputFile->GetStream());
  
  // If this command is the last to reference its output file, write the
  // footer information.
  if(m_OutputFile->LastReferencingCommandIs(this))
    {
    this->WriteConfigurationFooter(m_OutputFile->GetStream());
    }
}


/**
 * Write the CABIL configuration code to define this InstantiationSet.
 */
void cmCabilInstantiateCommand::WriteConfiguration(std::ostream& os) const
{
  os << std::endl
     << "  <InstantiationSet>" << std::endl;
  for(Elements::const_iterator e = m_Elements.begin();
      e != m_Elements.end(); ++e)
    {
    os << "    <Element>" << e->c_str() << "</Element>" << std::endl;
    }
  os << "  </InstantiationSet>" << std::endl;
}
