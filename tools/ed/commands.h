#ifndef COMMANDS_H
#define COMMANDS_H

enum ed_command
{
    a_append,
    c_change,
    d_delete,
    e_editfile,
    E_editfileunconditional,
    f_setfilename,
    g_global,
    G_globalinteractive,
    h_help,
    H_helptoggle,
    i_insert,
    j_join,
    k_mark,
    l_list,
    m_move,
    n_number,
    p_print,
    P_prompttoggle,
    q_quit,
    Q_quitunconditional,
    r_read,
    s_substitute,
    t_transfer,
    u_undo,
    v_globalnot,
    V_globalinteractivenot,
    w_write,
    wq_writequit,
    W_writeappend,
    x_paste,
    y_yank,
    z_scroll,
};

#endif
