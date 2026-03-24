// StringConditionExpression.cpp — 布尔表达式的递归下降解析器
#include "DPUFramework/StringConditionExpression.h"
#include "DPUFramework/DamageContext.h"

// ============================================================================
// 词法单元类型
// ============================================================================
namespace DPUCondition
{
	enum class ETokenType : uint8
	{
		Identifier,
		True,
		False,
		Number,
		And,       // &&
		Or,        // ||
		Not,       // !
		Eq,        // ==
		Neq,       // !=
		LParen,    // (
		RParen,    // )
		End
	};

	struct FToken
	{
		ETokenType Type;
		FString Value;
	};

	// ============================================================================
	// 词法分析器
	// ============================================================================
	static TArray<FToken> Tokenize(const FString& Expr)
	{
		TArray<FToken> Tokens;
		int32 Pos = 0;
		const int32 Len = Expr.Len();

		while (Pos < Len)
		{
			TCHAR Ch = Expr[Pos];

			// 跳过空白字符
			if (FChar::IsWhitespace(Ch))
			{
				Pos++;
				continue;
			}

			// 双字符运算符
			if (Pos + 1 < Len)
			{
				FString TwoChar = Expr.Mid(Pos, 2);
				if (TwoChar == TEXT("&&")) { Tokens.Add({ETokenType::And, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("||")) { Tokens.Add({ETokenType::Or, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("==")) { Tokens.Add({ETokenType::Eq, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("!=")) { Tokens.Add({ETokenType::Neq, TwoChar}); Pos += 2; continue; }
			}

			// 单字符运算符
			if (Ch == '!') { Tokens.Add({ETokenType::Not, TEXT("!")}); Pos++; continue; }
			if (Ch == '(') { Tokens.Add({ETokenType::LParen, TEXT("(")}); Pos++; continue; }
			if (Ch == ')') { Tokens.Add({ETokenType::RParen, TEXT(")")}); Pos++; continue; }

			// 数字字面量
			if (FChar::IsDigit(Ch) || (Ch == '-' && Pos + 1 < Len && FChar::IsDigit(Expr[Pos + 1])))
			{
				int32 Start = Pos;
				if (Ch == '-') Pos++;
				while (Pos < Len && (FChar::IsDigit(Expr[Pos]) || Expr[Pos] == '.'))
				{
					Pos++;
				}
				Tokens.Add({ETokenType::Number, Expr.Mid(Start, Pos - Start)});
				continue;
			}

			// 标识符或关键字 (true/false)
			if (FChar::IsAlpha(Ch) || Ch == '_')
			{
				int32 Start = Pos;
				while (Pos < Len && (FChar::IsAlnum(Expr[Pos]) || Expr[Pos] == '_'))
				{
					Pos++;
				}
				FString Word = Expr.Mid(Start, Pos - Start);
				if (Word == TEXT("true"))
				{
					Tokens.Add({ETokenType::True, Word});
				}
				else if (Word == TEXT("false"))
				{
					Tokens.Add({ETokenType::False, Word});
				}
				else
				{
					Tokens.Add({ETokenType::Identifier, Word});
				}
				continue;
			}

			// 未知字符 — 跳过
			Pos++;
		}

		Tokens.Add({ETokenType::End, TEXT("")});
		return Tokens;
	}

	// ============================================================================
	// 解析器（递归下降，即时求值）
	// ============================================================================
	struct FParser
	{
		const TArray<FToken>& Tokens;
		const UDamageContext* DC;
		int32 Pos;

		FParser(const TArray<FToken>& InTokens, const UDamageContext* InDC)
			: Tokens(InTokens), DC(InDC), Pos(0) {}

		const FToken& Current() const { return Tokens[Pos]; }
		void Advance() { if (Pos < Tokens.Num() - 1) Pos++; }

		bool ParseExpr() { return ParseOr(); }

		bool ParseOr()
		{
			bool Result = ParseAnd();
			while (Current().Type == ETokenType::Or)
			{
				Advance();
				bool Right = ParseAnd();
				Result = Result || Right;
			}
			return Result;
		}

		bool ParseAnd()
		{
			bool Result = ParseUnary();
			while (Current().Type == ETokenType::And)
			{
				Advance();
				bool Right = ParseUnary();
				Result = Result && Right;
			}
			return Result;
		}

		bool ParseUnary()
		{
			if (Current().Type == ETokenType::Not)
			{
				Advance();
				return !ParseUnary();
			}
			return ParsePrimary();
		}

		bool ParsePrimary()
		{
			// 括号表达式
			if (Current().Type == ETokenType::LParen)
			{
				Advance();
				bool Result = ParseExpr();
				if (Current().Type == ETokenType::RParen)
				{
					Advance();
				}
				return Result;
			}

			// 布尔字面量
			if (Current().Type == ETokenType::True)
			{
				Advance();
				return true;
			}
			if (Current().Type == ETokenType::False)
			{
				Advance();
				return false;
			}

			// 标识符 — 后面可能跟着 == 或 !=
			if (Current().Type == ETokenType::Identifier)
			{
				FDCValue LeftVal = DC ? DC->Get(FName(*Current().Value)) : FDCValue();
				Advance();

				// 检查是否有比较运算符
				if (Current().Type == ETokenType::Eq || Current().Type == ETokenType::Neq)
				{
					bool bIsEq = (Current().Type == ETokenType::Eq);
					Advance();

					FDCValue RightVal;
					if (Current().Type == ETokenType::True)
					{
						RightVal = FDCValue::FromBool(true);
						Advance();
					}
					else if (Current().Type == ETokenType::False)
					{
						RightVal = FDCValue::FromBool(false);
						Advance();
					}
					else if (Current().Type == ETokenType::Number)
					{
						RightVal = FDCValue::FromFloat(FCString::Atof(*Current().Value));
						Advance();
					}
					else if (Current().Type == ETokenType::Identifier)
					{
						RightVal = DC ? DC->Get(FName(*Current().Value)) : FDCValue();
						Advance();
					}

					bool bEqual = (LeftVal.AsBool() == RightVal.AsBool());
					return bIsEq ? bEqual : !bEqual;
				}

				// 裸标识符 — 作为布尔值求值
				return LeftVal.AsBool();
			}

			// 兜底 — 合法表达式不应到达此处
			Advance();
			return false;
		}
	};

	// ============================================================================
	// 字段提取器（扫描标识符，忽略关键字）
	// ============================================================================
	static TArray<FName> ExtractFields(const TArray<FToken>& Tokens)
	{
		TArray<FName> Fields;
		for (const FToken& Token : Tokens)
		{
			if (Token.Type == ETokenType::Identifier)
			{
				Fields.AddUnique(FName(*Token.Value));
			}
		}
		return Fields;
	}
}

// ============================================================================
// UStringConditionExpression 实现
// ============================================================================

bool UStringConditionExpression::Evaluate(const UDamageContext* DC) const
{
	if (Expression.IsEmpty())
	{
		return true; // 无条件 = 始终执行
	}

	TArray<DPUCondition::FToken> Tokens = DPUCondition::Tokenize(Expression);
	DPUCondition::FParser Parser(Tokens, DC);
	return Parser.ParseExpr();
}

TArray<FName> UStringConditionExpression::GetConsumedFields() const
{
	if (Expression.IsEmpty())
	{
		return {};
	}

	TArray<DPUCondition::FToken> Tokens = DPUCondition::Tokenize(Expression);
	return DPUCondition::ExtractFields(Tokens);
}

UStringConditionExpression* UStringConditionExpression::Create(UObject* Outer, const FString& InExpression)
{
	UStringConditionExpression* Obj = NewObject<UStringConditionExpression>(Outer);
	Obj->Expression = InExpression;
	return Obj;
}
