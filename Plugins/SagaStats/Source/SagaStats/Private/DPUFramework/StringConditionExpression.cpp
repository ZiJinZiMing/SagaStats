// StringConditionExpression.cpp — 布尔表达式的递归下降解析器
#include "DPUFramework/StringConditionExpression.h"
#include "DPUFramework/DamageContext.h"
#include "GameplayTagContainer.h"

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
		Gt,        // >
		Lt,        // <
		Gte,       // >=
		Lte,       // <=
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

			// 双字符运算符（优先匹配，避免 >= 被拆成 > 和 =）
			if (Pos + 1 < Len)
			{
				FString TwoChar = Expr.Mid(Pos, 2);
				if (TwoChar == TEXT("&&")) { Tokens.Add({ETokenType::And, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("||")) { Tokens.Add({ETokenType::Or, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("==")) { Tokens.Add({ETokenType::Eq, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("!=")) { Tokens.Add({ETokenType::Neq, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT(">=")) { Tokens.Add({ETokenType::Gte, TwoChar}); Pos += 2; continue; }
				if (TwoChar == TEXT("<=")) { Tokens.Add({ETokenType::Lte, TwoChar}); Pos += 2; continue; }
			}

			// 单字符运算符
			if (Ch == '!') { Tokens.Add({ETokenType::Not, TEXT("!")}); Pos++; continue; }
			if (Ch == '>') { Tokens.Add({ETokenType::Gt, TEXT(">")}); Pos++; continue; }
			if (Ch == '<') { Tokens.Add({ETokenType::Lt, TEXT("<")}); Pos++; continue; }
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
	// 辅助：判断 Token 是否为比较运算符
	// ============================================================================
	static bool IsCompareOp(ETokenType Type)
	{
		return Type == ETokenType::Eq || Type == ETokenType::Neq
			|| Type == ETokenType::Gt || Type == ETokenType::Lt
			|| Type == ETokenType::Gte || Type == ETokenType::Lte;
	}

	// ============================================================================
	// 类型感知比较
	// ============================================================================

	/** 相等比较：Bool/Name/Tag 做类型感知比较，其余走 AsFloat() */
	static bool CompareEqual(const FDCValue& L, const FDCValue& R)
	{
		// 两侧都是 Bool → 布尔比较
		if (L.Type == EDCValueType::Bool && R.Type == EDCValueType::Bool)
		{
			return L.BoolValue == R.BoolValue;
		}
		// 两侧都是 Name → 名称比较
		if (L.Type == EDCValueType::Name && R.Type == EDCValueType::Name)
		{
			return L.NameValue == R.NameValue;
		}
		// 任一侧是 Tag → Tag 比较（MatchesTagExact）
		if (L.Type == EDCValueType::Tag || R.Type == EDCValueType::Tag)
		{
			return L.AsTag().MatchesTagExact(R.AsTag());
		}
		// 其余情况（含 None）→ 数值比较
		return FMath::IsNearlyEqual(L.AsFloat(), R.AsFloat());
	}

	/** 数值比较（>, <, >=, <=）：统一走 AsFloat() */
	static bool CompareNumeric(const FDCValue& L, const FDCValue& R, ETokenType Op)
	{
		const float LF = L.AsFloat();
		const float RF = R.AsFloat();
		switch (Op)
		{
		case ETokenType::Gt:  return LF > RF;
		case ETokenType::Lt:  return LF < RF;
		case ETokenType::Gte: return LF >= RF || FMath::IsNearlyEqual(LF, RF);
		case ETokenType::Lte: return LF <= RF || FMath::IsNearlyEqual(LF, RF);
		default:              return false;
		}
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

		/** 解析右侧值：标识符（从DC读取）、布尔字面量、数字字面量 */
		FDCValue ParseValue()
		{
			if (Current().Type == ETokenType::True)
			{
				Advance();
				return FDCValue::FromBool(true);
			}
			if (Current().Type == ETokenType::False)
			{
				Advance();
				return FDCValue::FromBool(false);
			}
			if (Current().Type == ETokenType::Number)
			{
				FDCValue Val = FDCValue::FromFloat(FCString::Atof(*Current().Value));
				Advance();
				return Val;
			}
			if (Current().Type == ETokenType::Identifier)
			{
				FDCValue Val = DC ? DC->Get(FName(*Current().Value)) : FDCValue();
				Advance();
				return Val;
			}
			// 兜底
			Advance();
			return FDCValue();
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

			// 标识符 — 后面可能跟着比较运算符
			if (Current().Type == ETokenType::Identifier)
			{
				FDCValue LeftVal = DC ? DC->Get(FName(*Current().Value)) : FDCValue();
				Advance();

				// 检查是否有比较运算符
				if (IsCompareOp(Current().Type))
				{
					ETokenType Op = Current().Type;
					Advance();

					FDCValue RightVal = ParseValue();

					switch (Op)
					{
					case ETokenType::Eq:  return CompareEqual(LeftVal, RightVal);
					case ETokenType::Neq: return !CompareEqual(LeftVal, RightVal);
					default:              return CompareNumeric(LeftVal, RightVal, Op);
					}
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
