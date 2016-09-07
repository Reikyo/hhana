# import mva.regions
# import mva.categories.hadhad.common
import operator
from mva.samples import Data
from mva.samples import Higgs
from rootpy.tree import Cut
from mva.categories.hadhad import (
  Category_Preselection,
  Category_Cuts_VBF,
  Category_Cuts_VBF_LowDR,
  Category_Cuts_VBF_HighDR_Tight,
  Category_Cuts_VBF_HighDR_Loose,
  Category_Cuts_Boosted,
  Category_Cuts_Boosted_Tight,
  Category_Cuts_Boosted_Loose)
# This line works because of hadhad/__init__.py having cb.py where the category is based:
from mva.categories.hadhad import Category_Cuts_VBF_LowDR

year     = 2016
# mode     = 'Data'
mode     = 'MC'
# mode_mc  = 'gg'
mode_mc  = 'VBF'
# Weighting only affects MC:
weighted = False

if   mode == 'Data':
  cutflow_input = Data(year)
elif mode == 'MC':
  cutflow_input = Higgs(year, mode = mode_mc)

# -------------------------------------------------------------------------------------------------

# Preselection:

cuts_preselection = []

# GRL
# Removed from v14 ntuples onwards
# if mode == 'Data':
#   cuts_preselection.append(Cut('grl_pass_run_lb == 1'))

# MC ntuple splitting
# In mva/samples/sample.py
# Note that this cut is now standard and not part of preselection
# if mode == 'MC':
#   if   year == 2015:
#     cuts_preselection.append(Cut('NOMINAL_pileup_random_run_number <  284490'))
#   elif year == 2016:
#     cuts_preselection.append(Cut('NOMINAL_pileup_random_run_number >= 284490'))

# Trigger
# In mva/categories/triggers.py
# Note that this cut is now standard and not part of preselection
# if   year == 2015:
#   cuts_preselection.append(Cut('HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM == 1'))
# elif year == 2016:
#   cuts_preselection.append(Cut('HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo == 1'))

# Jet trigger
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('(jet_0_pt > 70.0) && (abs(jet_0_eta) < 3.2)')) # okay when fabs changed to abs
# Charge
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('(abs(ditau_tau0_q) == 1) && (abs(ditau_tau1_q) == 1)'))
# ntracks
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut(   '((ditau_tau0_n_tracks == 1) || (ditau_tau0_n_tracks == 3)) ' +
                             '&& ((ditau_tau1_n_tracks == 1) || (ditau_tau1_n_tracks == 3))')) # okay when more brackets added
# Tau ID
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut(   '(n_taus_medium == 2) && (n_taus_tight >= 1) ' +
                             '&& (ditau_tau0_jet_bdt_medium == 1) && (ditau_tau1_jet_bdt_medium == 1)')) # okay when "==1" added for last two conditions
# Tau pT
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('(ditau_tau0_pt > 40) && (ditau_tau1_pt > 30)'))
# OS
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('selection_opposite_sign == 1'))
# MET
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('selection_met == 1'))
# MET centrality
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('selection_met_centrality == 1'))
# deta
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('selection_delta_eta == 1'))
# dr
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('selection_delta_r == 1'))
# Lepton veto (no BDT)
# In mva/categories/hadhad/common.py
cuts_preselection.append(Cut('selection_lepton_veto == 1'))

# -------------------------------------------------------------------------------------------------

# Cut-Based VBF:

cuts_cutbased_vbf = []

# Sublead Jet
cuts_cutbased_vbf.append(Cut('jet_1_pt > 30'))
# Mjj
cuts_cutbased_vbf.append(Cut('dijet_vis_mass > 300'))
# Delta Eta Jets
cuts_cutbased_vbf.append(Cut('dijet_deta > 3.0'))
# Eta Product
cuts_cutbased_vbf.append(Cut(   '((jet_0_eta < 0) && (jet_1_eta > 0)) ' +
                             '|| ((jet_0_eta > 0) && (jet_1_eta < 0))'))
# Tau 1 Topology
cuts_cutbased_vbf.append(Cut(   '((jet_0_eta < ditau_tau0_eta) && (ditau_tau0_eta < jet_1_eta)) ' +
                             '|| ((jet_1_eta < ditau_tau0_eta) && (ditau_tau0_eta < jet_0_eta))'))
# Tau 2 Topology
cuts_cutbased_vbf.append(Cut(   '((jet_0_eta < ditau_tau1_eta) && (ditau_tau1_eta < jet_1_eta)) ' +
                             '|| ((jet_1_eta < ditau_tau1_eta) && (ditau_tau1_eta < jet_0_eta))'))
# Delta Eta Taus
cuts_cutbased_vbf.append(Cut('ditau_deta < 1.5'))

# VBF Cut-Based HighPT
HH16_VBF_LOWDR         = Cut('(ditau_higgs_pt >= 140) && (ditau_dr <= 1.5)')

HH16_VBF_HIGHDR_DITAU  = Cut('(ditau_higgs_pt <  140) || (ditau_dr >  1.5)')
HH16_VBF_HIGHDR_DIJET1 = Cut('dijet_vis_mass > (-250 * dijet_deta + 1550)')
HH16_VBF_HIGHDR_DIJET2 = Cut('dijet_vis_mass < (-250 * dijet_deta + 1550)')
# VBF Cut-Based LowPT - Tight
HH16_VBF_HIGHDR_TIGHT  = HH16_VBF_HIGHDR_DITAU & HH16_VBF_HIGHDR_DIJET1
# VBF Cut-Based LowPT - Loose
HH16_VBF_HIGHDR_LOOSE  = HH16_VBF_HIGHDR_DITAU & HH16_VBF_HIGHDR_DIJET2

HH16_CBA_VBF           = HH16_VBF_LOWDR | HH16_VBF_HIGHDR_TIGHT | HH16_VBF_HIGHDR_LOOSE

# -------------------------------------------------------------------------------------------------

# Cut-Based Boosted:

cuts_cutbased_boosted = []

# Higgs pT
cuts_cutbased_boosted.append(Cut('ditau_higgs_pt > 100'))
# Delta Eta Taus
cuts_cutbased_boosted.append(Cut('ditau_deta < 1.5'))

# Boosted Cut-Based High PT
HH16_BOOST_TIGHT = Cut('(ditau_higgs_pt >= 140) && (ditau_dr <= 1.5)')
# Boosted Cut-Based Low PT
HH16_BOOST_LOOSE = Cut('(ditau_higgs_pt <  140) || (ditau_dr >  1.5)')

HH16_CBA_BOOST   = HH16_BOOST_TIGHT | HH16_BOOST_LOOSE

# -------------------------------------------------------------------------------------------------

cuts = cuts_preselection

# cuts_cutbased_vbf.append(HH16_CBA_VBF)
# cuts_cutbased_vbf.append(HH16_VBF_LOWDR)
# cuts_cutbased_vbf.append(HH16_VBF_HIGHDR_TIGHT)
# cuts_cutbased_vbf.append(HH16_VBF_HIGHDR_LOOSE)

# cuts_cutbased_boosted.append(HH16_CBA_BOOST)
# cuts_cutbased_boosted.append(HH16_BOOST_TIGHT)
# cuts_cutbased_boosted.append(HH16_BOOST_LOOSE)

# cuts = cuts + cuts_cutbased_vbf
# cuts = cuts + cuts_cutbased_boosted

# -------------------------------------------------------------------------------------------------

# Without cuts (except those now hard-wired in the actual hhana code):
print cutflow_input.events(weighted = weighted)[1].value

# With cuts:
print cutflow_input.events(weighted = weighted, cuts = reduce(operator.and_, cuts))[1].value

# With cuts added one by one:
# cutflow_output = []
# for i in range(len(cuts)):
#   cutflow_output.append(cutflow_input.events(weighted = weighted, cuts = reduce(operator.and_, cuts[0:i+1]))[1].value)
# for i in cutflow_output:
#   print i

# print 'From the category:'
# print cutflow_input.events(weighted = weighted, category = Category_Preselection)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_VBF)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_VBF_LowDR)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_VBF_HighDR_Tight)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_VBF_HighDR_Loose)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_Boosted)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_Boosted_Tight)[1].value
# print cutflow_input.events(weighted = weighted, category = Category_Cuts_Boosted_Loose)[1].value

# -------------------------------------------------------------------------------------------------

# v12 (all MC weighted => fractional values)
#                         2015 MC 1       2016 MC 1       2016 MC 2       2016 MC 3       TWiki   2016 Data 1   2016 Data 2   TWiki
# hh16_preselection                                        9.4663028717    9.4664030075    8.05   3690          3690          3690
# hh16_CBA_vbf             1.8114967346    4.3541994094    4.4833865165    4.4833865165    3.81    200           200           200
# hh16_vbf_lowdr           1.0301972627    2.4898321628    2.5637075901    2.5637075901    2.18     60            60            60
# hh16_vbf_highdr_tight    0.5888065695    1.4254974126    1.4677808284    1.4677808284    1.25     71            71            71
# hh16_vbf_highdr_loose    0.1924895048    0.4389009475    0.4518596231    0.4518596231    0.38     69            69            69
# hh16_CBA_boost           3.2805538177    8.0066175460    8.2450389862    4.2035040855    3.57   2526          2378          2378
# hh16_boost_tight         1.9652493000    4.7694296836    4.9111442565    2.3474190235    2.00    934           874           874
# hh16_boost_loose         1.3153077364    3.2377154827    3.3339385986    1.8560403585    1.58   1592          1504          1504

# v14 (all MC weighted => fractional values)
#                         2016 MC 1       2016 MC 2       2016 MC 3       2016 MC 4       TWiki   2016 Data 1   2016 Data 2   2016 Data 3   TWiki
#                                         2016/08/25      2016/08/26      2016/09/01              2016/08/26    2016/09/01
# hh16_preselection       11.5462884903   11.5454444885   11.5454444885   11.2639646530   16.60   7361          7325          7639          7639
# hh16_CBA_vbf             5.4178514481    5.4178514481    5.4178514481    5.3607172966    7.89    398           397           414           414
# hh16_vbf_lowdr           2.9940612316    2.9940612316    2.9940612316    3.0650110245    4.48    117           116           122           122
# hh16_vbf_highdr_tight    1.8344278336    1.8344278336    1.8344278336    1.7478357554    2.61    142           142           146           146
# hh16_vbf_highdr_loose    0.5893480778    0.5893480778    0.5893480778    0.5478137136    0.80    139           139           146           146
# hh16_CBA_boost           5.1471672058    5.1463232040    5.1463232040    5.0084547997    7.37   4713          4682          4890          4890
# hh16_boost_tight         2.8209977150    2.8201534748    2.8201534748    2.8118076325    4.11   1723          1711          1773          1773
# hh16_boost_loose         2.3261735439    2.3261735439    2.3261735439    2.1965723038    3.26   2990          2971          3117          3117

# -------------------------------------------------------------------------------------------------

# cuts.append(Cut(''))
# cuts.append(mva.regions.Q)
# cuts.append(mva.regions.P1P3)
# cuts.append(mva.regions.ID_MEDIUM)
# cuts.append(mva.regions.TRACK_ISOLATION)
# cuts.append(mva.categories.hadhad.common.LEAD_TAU_40)
# cuts.append(mva.categories.hadhad.common.SUBLEAD_TAU_30)
# cuts.append(mva.regions.OS)
# cuts.append(mva.categories.hadhad.common.MET)
# # cuts.append(mva.categories.hadhad.common.MET_CENTRALITY)
# cuts.append(mva.categories.hadhad.common.DETA_TAUS)
# cuts.append(mva.categories.hadhad.common.DR_TAUS)

# cuts0_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0]))[1].value
# cuts1_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1]))[1].value
# cuts2_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2]))[1].value
# cuts3_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3]))[1].value
# cuts4_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4]))[1].value
# cuts5_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5]))[1].value
# cuts6_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6]))[1].value
# cuts7_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7]))[1].value
# cuts8_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8]))[1].value
# cuts9_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9]))[1].value
# cuts10_out = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9] & cuts[10]))[1].value
# cuts11_out = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9] & cuts[10] & cuts[11]))[1].value
# cuts12_out = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9] & cuts[10] & cuts[11] & cuts[12]))[1].value
