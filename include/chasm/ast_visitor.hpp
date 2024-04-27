#ifndef CHASM_VISITOR_HPP
#define CHASM_VISITOR_HPP


namespace chasm::ast
{
	struct procedure_statement;
	struct instruction_statement;
	struct define_statement;
	struct config_statement;
	struct sprite_statement;
	struct raw_statement;
	struct label_statement;

	struct base_visitor
	{
		virtual void visit(const procedure_statement&) {};
		virtual void visit(const instruction_statement&) {};
		virtual void visit(const define_statement&) {};
		virtual void visit(const config_statement&) {};
		virtual void visit(const sprite_statement&) {};
		virtual void visit(const raw_statement&) {};
		virtual void visit(const label_statement&) {};
	};
}


#endif //chasm_VISITOR_HPP
