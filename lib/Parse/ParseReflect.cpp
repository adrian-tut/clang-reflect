//===--- Parser.cpp - C Language Family Parser ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements parsing for reflection facilities.
//
//===----------------------------------------------------------------------===//

#include "clang/Parse/Parser.h"
#include "clang/AST/ASTContext.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Parse/RAIIObjectsForParser.h"
using namespace clang;


/// Parse a reflect-expression.
///
/// \verbatim
///       reflect-expression:
///         'reflexpr' '(' id-expression ')'
///         'reflexpr' '(' type-id ')'
///         'reflexpr' '(' namespace-name ')'
/// \endverbatim
ExprResult Parser::ParseCXXReflectExpression() {
  assert(Tok.is(tok::kw_reflexpr) && "expected 'reflexpr'");
  SourceLocation KWLoc = ConsumeToken();

  BalancedDelimiterTracker T(*this, tok::l_paren);
  if (T.expectAndConsume(diag::err_expected_lparen_after, "reflexpr"))
    return ExprError();
  
    unsigned Kind; // The reflected entity kind.
    ParsedReflectionPtr Entity; // The actual entity.

  // FIXME: The operand parsing is a bit fragile. We could do a lot better
  // by looking at tokens to characterize the parse before committing.
  //
  // Also, is it possible to reflect expressions within this framework?

  CXXScopeSpec SS;
  ParseOptionalCXXScopeSpecifier(SS, nullptr, /*EnteringContext=*/false);

  // If the next token is an identifier, try to resolve that. This will likely
  // match most uses of the reflection operator, but there are some cases
  // of id-expressions and type-ids that must be handled separately.
  //
  // FIXME: This probably won't work for things operator and conversion
  // functions. 
  if (!SS.isInvalid() && Tok.is(tok::identifier)) {
    IdentifierInfo *Id = Tok.getIdentifierInfo();
    SourceLocation IdLoc = ConsumeToken();

    if (!Actions.ActOnReflectedId(SS, IdLoc, Id, Kind, Entity))
      return ExprError();
  }
  else if (isCXXTypeId(TypeIdAsTemplateArgument)) {
    DeclSpec DS(AttrFactory);
    ParseSpecifierQualifierList(DS);
    Declarator D(DS, Declarator::TypeNameContext);
    ParseDeclarator(D);
    if (D.isInvalidType())
      return ExprError();
    if (!Actions.ActOnReflectedType(D, Kind, Entity))
      return ExprError();
  }

  if (T.consumeClose())
    return ExprError();
  
  return Actions.ActOnCXXReflectExpression(KWLoc, Kind, Entity, 
                                     T.getOpenLocation(), 
                                     T.getCloseLocation());
}

static ReflectionTrait ReflectionTraitKind(tok::TokenKind kind) {
  switch (kind) {
  default:
    llvm_unreachable("Not a known type trait");
#define REFLECTION_TRAIT_1(Spelling, K)                                        \
  case tok::kw_##Spelling:                                                     \
    return URT_##K;
#define REFLECTION_TRAIT_2(Spelling, K)                                        \
  case tok::kw_##Spelling:                                                     \
    return BRT_##K;
#include "clang/Basic/TokenKinds.def"
  }
}

static unsigned ReflectionTraitArity(tok::TokenKind kind) {
  switch (kind) {
  default:
    llvm_unreachable("Not a known type trait");
#define REFLECTION_TRAIT(N, Spelling, K)                                       \
  case tok::kw_##Spelling:                                                     \
    return N;
#include "clang/Basic/TokenKinds.def"
  }
}

/// \brief Parse a reflection trait.
///
/// \verbatim
///   primary-expression:
///     unary-reflection-trait '(' expression ')'
///     binary-reflection-trait '(' expression ',' expression ')'
///
///   unary-reflection-trait:
///     '__reflect_index'
/// \endverbatim
ExprResult Parser::ParseReflectionTrait() {
  tok::TokenKind Kind = Tok.getKind();
  SourceLocation Loc = ConsumeToken();

  // Parse any number of arguments in parens.
  BalancedDelimiterTracker Parens(*this, tok::l_paren);
  if (Parens.expectAndConsume())
    return ExprError();
  SmallVector<Expr *, 2> Args;
  do {
    ExprResult Expr = ParseConstantExpression();
    if (Expr.isInvalid()) {
      Parens.skipToEnd();
      return ExprError();
    }
    Args.push_back(Expr.get());
  } while (TryConsumeToken(tok::comma));
  if (Parens.consumeClose())
    return ExprError();
  SourceLocation RPLoc = Parens.getCloseLocation();

  // Make sure that the number of arguments matches the arity of trait.
  unsigned Arity = ReflectionTraitArity(Kind);
  if (Args.size() != Arity) {
    Diag(RPLoc, diag::err_type_trait_arity)
        << Arity << 0 << (Arity > 1) << (int)Args.size() << SourceRange(Loc);
    return ExprError();
  }

  ReflectionTrait Trait = ReflectionTraitKind(Kind);
  return Actions.ActOnReflectionTrait(Loc, Trait, Args, RPLoc);
}

