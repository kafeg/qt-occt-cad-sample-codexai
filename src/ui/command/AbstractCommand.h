#pragma once

#include <Standard_TypeDef.hxx>

class Document;

// Simple command interface operating on the model Document
class AbstractCommand
{
public:
  virtual ~AbstractCommand() = default;
  virtual void execute(Document& doc) = 0;
};

