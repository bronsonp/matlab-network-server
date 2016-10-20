classdef HardToSerialise
    properties(SetAccess = immutable)
        % This class is hard to serialise because of the immutable property
        % here that restricts access from third party serialisation code.
        number
    end
    
    methods
        function self = HardToSerialise(self)
            self.number = rand();
        end
    end
end
