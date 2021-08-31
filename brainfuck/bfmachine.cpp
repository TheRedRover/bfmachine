#include <stack>
#include "bfmachine.hpp"

void cmd::nxt(std::unique_ptr<cmd> nxt)
{
    nxt_ = std::move(nxt);
}

cmd *cmd::get_nxt()
{
    return nxt_.get();
}

void cmd::fn(){ nxt_->fn(); }

void decc::fn()
{
    buf[*head]-=am;
    if(nxt_)
        nxt_->fn();
}

decc::decc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}

void incc::fn()
{
    buf[*head]+=am;
    if(nxt_)
        nxt_->fn();
}

incc::incc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}


void mvlc::fn()
{
    if(((*head)-=am)<0)
        throw std::logic_error("");
    if(nxt_)
        nxt_->fn();
}

mvlc::mvlc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}

void mvrc::fn()
{
    std::cout<<"Moving right\n";
    if(((*head)+=am)<0)
        throw std::logic_error("");
    if(nxt_)
        nxt_->fn();
}

mvrc::mvrc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}

void blc::fn()
{
    if(!nxt_)
        std::cout<<"BLC - NEXT POINTER IS NULLPTR\n";
    if(!elc)
        std::cout<<"BLC - BEGIN LOOP POINTER IS NULLPTR\n";
    if(buf[*head]==0)
        elc->fn();
    else
        nxt_->fn();

}

void blc::set_elc(cmd *elc_)
{
    elc = elc_;
}

blc::blc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}

void elc::fn()
{
    if(!nxt_)
        std::cout<<"ELC - NEXT POINTER IS NULLPTR\n";
    if(!blc)
        std::cout<<"ELC - BEGIN LOOP POINTER IS NULLPTR\n";
    if(buf[*head]==0)
    {
        if (nxt_)
            nxt_->fn();
    }
    else
        blc->fn();

}

void elc::set_blc(cmd *blc_)
{
    blc_ = blc_;
}

elc::elc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}

void outc::fn()
{

    for(auto i = 0; i < am; ++i)
        std::cout<<buf[*head];
    if (nxt_)
        nxt_->fn();
}

outc::outc( std::shared_ptr<int> head, char *buf, int am) : cmd(head, buf, am) {}

std::vector<std::pair<char, size_t>> bfmachine::s_to_ps( const std::string &str) {
    if(str.empty())
        throw std::invalid_argument("Code string is empty");

    std::vector<std::pair<char, size_t>> sps;
    size_t c = 1;
    for(auto i = ++str.begin(); i!=++str.end(); ++i)
    {
        if((*i)==*(i-1)&&(*i)!=LEFT_BRACKET&&(*i)!=RIGHT_BRACKET)
            ++c;
        else
        {
            sps.push_back(std::make_pair(*(i-1),c));
            c=1;
        }
    }
    return sps;
}

void bfmachine::init( const std::string &str)
{
    std::shared_ptr<int> head_ptr = std::make_shared<int>(head);


    std::stack<std::unique_ptr<elc>> stack;
    auto ps = s_to_ps(str);
    if(ps.empty())
        throw std::logic_error("There isn't bfmachine code in string/file");
    cmd * current_ptr;
    first_cmd = std::make_unique<cmd>(cmd(head_ptr,cpu_first,1));
    current_ptr = first_cmd.get();
    for(auto  p:ps)
    {
        switch (p.first) {
            case MINUS: {
                current_ptr->nxt(std::make_unique<decc>(head_ptr, cpu_first, p.second));
                current_ptr = current_ptr->get_nxt();
                break;
            }
            case PLUS: {
                current_ptr->nxt(std::make_unique<incc>(head_ptr, cpu_first, p.second));
                current_ptr = current_ptr->get_nxt();
                break;
            }
            case LEFT: {
                current_ptr->nxt(std::make_unique<mvlc>(head_ptr, cpu_first, p.second));
                current_ptr = current_ptr->get_nxt();
                break;
            }
            case RIGHT: {
                current_ptr->nxt(std::make_unique<mvrc>(head_ptr, cpu_first, p.second));
                current_ptr = current_ptr->get_nxt();
                break;
            }
            case LEFT_BRACKET: {
                auto blc_ = std::make_unique<blc>(head_ptr, cpu_first, p.second);
                auto elc_ = std::make_unique<elc>(head_ptr, cpu_first, p.second);
                blc_->set_elc(elc_.get());
                stack.push(std::move(elc_));
                current_ptr->nxt(std::move(blc_));
                current_ptr = current_ptr->get_nxt();
                stack.top()->set_blc(current_ptr);
                break;
            }
            case RIGHT_BRACKET: {
                if (stack.empty())
                    throw std::logic_error("Left bracket '[' is missing");
                if(!stack.top()->blc)
                    std::cout<<"LOOOOOOOK!!!\n";
                current_ptr->nxt(std::move(stack.top()));

                current_ptr = current_ptr->get_nxt();
                stack.pop();
                break;
            }
            case POINT:
            {
                current_ptr->nxt(std::make_unique<outc>(head_ptr, cpu_first, p.second));
                current_ptr = current_ptr->get_nxt();
                break;
            }
            default:
                break;
        }
    }
    if(!stack.empty())
        throw std::logic_error("Right bracket ']' is missing");
}

void bfmachine::execute()
{
    if(first_cmd)
        first_cmd->fn();
}
