"""Learner host-side audit scaffold; implement the bounded bundle contract.

The target remains C17. Python/binary tools are outside its runtime closure.
Return a list of rejection reasons; [] is an acceptance assertion. Missing or
unsupported evidence must reject. See CONTRACTS.md for the selected ELF scope.
"""
def validate(bundle,expected_binary_sha256,expected_policy_sha256):
    raise NotImplementedError('Implement source/configuration/binary/report binding and origin checks')
