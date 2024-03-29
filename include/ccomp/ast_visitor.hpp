#ifndef CCOMP_VISITOR_HPP
#define CCOMP_VISITOR_HPP


namespace ccomp::ast
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


#endif //CCOMP_VISITOR_HPP
