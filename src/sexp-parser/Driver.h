/* -*- C++ -*- */
/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Defines a driver class to lex/parse filter s-expressions */

#ifndef DECOMPRESSOR_PROTOTYPE_SRC_SEXP_PARSER_DRIVER_H
#define DECOMPRESSOR_PROTOTYPE_SRC_SEXP_PARSER_DRIVER_H

#include "sexp-parser/Parser.tab.hpp"
#include "utils/Allocator.h"

#include <map>
#include <memory>
#include <string>

namespace wasm {
namespace filt {

// Driver connecting Parser, Lexer, Positions, and Locations.
class Driver {
  Driver(const Driver&) = delete;
  Driver& operator=(const Driver&) = delete;

 public:
  Driver(std::shared_ptr<SymbolTable> Table)
      : Table(Table),
        TraceLexing(false),
        TraceParsing(false),
        ParsedAst(nullptr),
        ErrorsReported(false) {}

  ~Driver() {}

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    return Table->create<T>(std::forward<Args>(args)...);
  }

#define X(tag, format, defval, mergable, NODE_DECLS)               \
  tag##Node* get##tag##Definition(                                 \
      decode::IntType Value,                                       \
      decode::ValueFormat Format = decode::ValueFormat::Decimal) { \
    return Table->get##tag##Definition(Value, Format);             \
  }                                                                \
  tag##Node* get##tag##Definition() { return Table->get##tag##Definition(); }
  AST_INTEGERNODE_TABLE
#undef X

  SymbolNode* getSymbolDefinition(ExternalName& Name) {
    return Table->getSymbolDefinition(Name);
  }

  // The name of the file being parsed.
  std::string& getFilename() { return Filename; }

  void setTraceLexing(bool NewValue) { TraceLexing = NewValue; }

  void setTraceParsing(bool NewValue) { TraceParsing = NewValue; }

  const location& getLoc() const { return Loc; }

  void stepLocation() { Loc.step(); }

  void extendLocationColumns(size_t NumColumns) { Loc.columns(NumColumns); }

  void extendLocationLines(size_t NumLines) { Loc.lines(NumLines); }

  // Run the parser on file F. Returns true on success.
  bool parse(const std::string& Filename);

  // Returns the last parsed ast.
  Node* getParsedAst() const { return ParsedAst; }

  void setParsedAst(Node* Ast) {
    ParsedAst = Ast;
    Table->install(ParsedAst);
  }

  // Error handling.
  void error(const wasm::filt::location& Loc, const std::string& Message);
  void error(const std::string& Message);
  void tokenError(const std::string& Token);
  void fatal(const std::string& Message);

 private:
  std::shared_ptr<SymbolTable> Table;
  std::string Filename;
  bool TraceLexing;
  bool TraceParsing;
  bool MaintainIntegerFormatting;
  // The location of the last token.
  location Loc;
  Node* ParsedAst;
  bool ErrorsReported;

  // Called before parsing for setup.
  void Begin();
  // Called after parsing for cleanup.
  void End();
};

}  // end of namespace filt

}  // end of namespace filt

// Tell Flex the lexer's prototype ...
#define YY_DECL \
  wasm::filt::Parser::symbol_type yylex(wasm::filt::Driver& Driver)
// ... and declare it for the parser's sake.
YY_DECL;

#endif  // DECOMPRESSOR_PROTOTYPE_SRC_SEXP_PARSER_DRIVER_H
