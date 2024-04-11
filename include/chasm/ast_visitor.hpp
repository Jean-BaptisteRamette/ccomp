#ifndef CHASM_VISITOR_HPP
#define CHASM_VISITOR_HPP


namespace chasm::ast
{
	struct procedure_statement;
	struct instruction_statement;
	struct define_statement;
	struct sprite_statement;
	struct raw_statement;
	struct label_statement;

	struct base_visitor
	{
		virtual void visit(const procedure_statement&) = 0;
		virtual void visit(const instruction_statement&) = 0;
		virtual void visit(const define_statement&) = 0;
		virtual void visit(const sprite_statement&) = 0;
		virtual void visit(const raw_statement&) = 0;
		virtual void visit(const label_statement&) = 0;
	};
}


#endif //chasm_VISITOR_HPP
